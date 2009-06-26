/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../../config.h"
#include "../cfg.h"
#include "../mem.h"
#include "../net.h"
#include "../util.h"
#include "mpdscrob.h"
#include "extra/md5.h"

#define SCROB_CLIENT "tst"
#define SCROB_VERSION "1.0"
#define SCROB_RETRIES 3

/* Reminder: implement the caching feature while local/remote network connection
 * is down.  Most clients seem to write to a cache file, might be better for
 * us to simply store in a queue data structure. */

/* Globals. */
static char last_artist[128];
static char last_title[128];
static char last_album[128];
static char last_track[16];
static int last_etime;
static int last_ttime;
int scrob_enabled;      /* bool */
static const char *scrob_host;
static int scrob_port;
static const char *scrob_user;
static const char *scrob_pass;
static struct sockaddr_in server;
static struct hostent *hptr;
static int scrob_shaked;        /* bool */
static char scrob_sessionid[36];
static char scrob_nowplayurl[256];
static char scrob_submiturl[256];

/* Function prototypes. */
static int scrob_connect(void);
static void scrob_submit(int sock, const char *artist, const char *title,
                         const char *album, const char *track, const int ttime);
static void scrob_nowplay(int sock, const char *artist, const char *title,
                          const char *album, const char *track, const int ttime);
static int scrob_handshake(int sock);
static void scrob_md5(char *str);
static char *scrob_urlenc(const char *str);
static int scrob_handshake_parse(char *buf);
static char *scrob_utime(char *dst, size_t sz);
static void scrob_send_submission(int sock, const char *snd, const char *url);

/**
 * @brief Called with mpd's init_settings method.
 */
void scrob_init(void)
{
        last_artist[0] = '\0';
        last_title[0] = '\0';
        last_etime = 0;
        last_ttime = 0;

        scrob_shaked = 0;
        scrob_sessionid[0] = '\0';

        scrob_host = get_char_key("mpd", "scrob_host", NULL);
        scrob_port = get_int_key("mpd", "scrob_port", 80);
        scrob_enabled = get_bool_key("mpd", "scrob_enabled", 0);
        scrob_user = get_char_key("mpd", "scrob_user", NULL);
        scrob_pass = get_char_key("mpd", "scrob_pass", NULL);

        if (scrob_host == NULL)
                scrob_enabled = 0;

        if (scrob_enabled) {
                if ((hptr = gethostbyname(scrob_host)) == NULL) {
                        fprintf(stderr,
                                "mpdscrob: Could not gethostbyname(%s)\n",
                                scrob_host);
                        scrob_enabled = 0;
                        return;
                }

                memcpy(&server.sin_addr, hptr->h_addr_list[0], hptr->h_length);
                server.sin_family = AF_INET;
                server.sin_port = htons((short) scrob_port);
        }
}

/**
 * @brief Figure out what to do, either send the now playing or the track
 *        submission stuff.
 *
 * @param artist
 * @param title
 * @param album
 * @param track
 * @param etime
 * @param ttime
 */
void scrob_urself(const char *artist, const char *title, const char *album,
                  const char *track, const int etime, const int ttime)
{
        int sock;

        /* Same effin' track. */
        if (!strcmp(last_artist, artist) && !strcmp(last_title, title)) {
                last_etime = etime;
                return;
        }

        /* Connect to the scrobbler server. */
        sock = scrob_connect();
        if (sock == -1) {
                fprintf(stderr,
                        "mpdscrob: Could not connect to scrobbler server ;[\n");
                
                goto ABOLISHTHECLASSSYSTEM;
        }

        /* We haven't done the handshake. */
        if (!scrob_shaked) {
                if ((scrob_handshake(sock) == -1)) {
                        DEBUGF(("mpdscrob: Handshake failed!\n"));

                        /* We need to update these variables so that we will only try
                         * to connect once per song, as to not spam! */
                        strfcpy(last_artist, artist, sizeof(last_artist));
                        strfcpy(last_title, title, sizeof(last_title));
                        
                        goto ABOLISHTHECLASSSYSTEM;
                }

                scrob_shaked = 1;
        }

        /* Okay, the last artist wasn't null and they played half or more
         * of the track, OR 240 seconds or more, that is a candidate for a
         * track to submit. */
        if (last_artist[0] != '\0' &&
            ((last_etime * 2) >= last_ttime || last_etime >= 240))
                scrob_submit(sock, last_artist, last_title, last_album,
                             last_track, last_ttime);

        /* Send now playing. */
        if (artist[0] != '\0' && title[0] != '\0')
                scrob_nowplay(sock, artist, title, album, track, ttime);

ABOLISHTHECLASSSYSTEM:
        /* Update all the last_* stuff. */
        strfcpy(last_artist, artist, sizeof(last_artist));
        strfcpy(last_title, title, sizeof(last_title));
        strfcpy(last_album, album, sizeof(last_album));
        strfcpy(last_track, track, sizeof(last_track));
        last_ttime = ttime;

        /* Close this maw faw. */
        if (sock)
                close(sock);
}

/**
 * @brief Do da hand shake f00.
 *
 * @param sock Socket
 *
 * @return Error status
 */
static int scrob_handshake(int sock)
{
        char utm[96];
        char utm_md5[160];
        int n;
        char buf[1024];

        DEBUGF(("mpdscrob: Handshaking with (%s:%d)!\n",
                scrob_host, scrob_port));

        scrob_utime(utm, sizeof(utm));

        strfcpy(utm_md5, scrob_pass, sizeof(utm_md5));
        strfcat(utm_md5, utm, sizeof(utm_md5));

        scrob_md5(utm_md5);

        sendcrlf(sock,
                 "GET http://%s/"
                 "?hs=true&p=1.2&c=%s&v=%s&u=%s&t=%s&a=%s\n"
                 "Host: %s\r\n",
                 scrob_host,
                 SCROB_CLIENT,
                 SCROB_VERSION,
                 scrob_user,
                 utm,
                 utm_md5,
                 scrob_host);

        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0) {
                fprintf(stderr, "mpdscrob: Socket disconnected!\n");
                return -1;
        }

        buf[n] = '\0';

        return scrob_handshake_parse(buf);
}

/**
 * @brief Parse the handshake reply.
 *
 * @param buf Buffer
 *
 * @return -1 for fail, 0 for success
 */
static int scrob_handshake_parse(char *buf)
{
        char *line;
        char *header;

        header = strstr(buf, "\r\n\r\n");
        
        if (header) {
                header += 4;
                buf = header;
        }

        DEBUGF(("mpdscrob buf: {\n%s\n}\n", buf));

        /* Okay, the buffer should contain 4 lines. */
        line = strtok(buf, "\n");
        if (line == NULL)
                return -1;

        if (strcmp(line, "OK") != 0)
                return -1;

        /* Session ID */
        line = strtok(NULL, "\n");
        if (line == NULL)
                return -1;

        strfcpy(scrob_sessionid, line, sizeof(scrob_sessionid));

        /* Now playing URL */
        line = strtok(NULL, "\n");
        if (line == NULL)
                return -1;

        strfcpy(scrob_nowplayurl, line, sizeof(scrob_nowplayurl));

        /* Submit URL */
        line = strtok(NULL, "\n");
        if (line == NULL)
                return -1;

        strfcpy(scrob_submiturl, line, sizeof(scrob_submiturl));
        return 0;
}

/**
 * @brief Get the md5 hash of a string.
 *
 * @param str String to motha effin hash (this will also be the storage, so
 *            make sure it is at least 32 + 1 bytes long.)
 */
static void scrob_md5(char *str)
{
        md5_state_t state;
        md5_byte_t digest[16];
        int i;

        md5_init(&state);
        md5_append(&state, (const md5_byte_t *) str, strlen(str));
        md5_finish(&state, digest);

        for (i = 0; i < 16; i++)
                sprintf(str + i * 2, "%02x", digest[i]);

        str[32] = '\0';
}

/**
 * @brief UNIX time shiznazzlage. */
static char *scrob_utime(char *dst, size_t sz)
{
        unsigned long int utime;

        utime = get_unix_time();
        uint_to_str(dst, utime, sz);

        return dst;
}

/**
 * @brief Submit track to scrobbler.
 *
 * @param artist
 * @param title
 * @param album
 * @param track
 * @param ttime
 */
static void scrob_submit(int sock, const char *artist, const char *title,
                         const char *album, const char *track, const int ttime)
{
        char utm[96];
        char snd[1024];
        char sttime[32];

        DEBUGF(("Submitting (%s - %s) to (%s)!\n",
                artist, title, scrob_submiturl));

        scrob_utime(utm, sizeof(utm));

        uint_to_str(sttime, ttime, sizeof(sttime));

        /* Assemble request string. */
        strfcpy(snd, "s=", sizeof(snd));
        strfcat(snd, scrob_sessionid, sizeof(snd));

        strfcat(snd, "&a[0]=", sizeof(snd));
        strfcat(snd, scrob_urlenc(artist), sizeof(snd));

        strfcat(snd, "&t[0]=", sizeof(snd));
        strfcat(snd, scrob_urlenc(title), sizeof(snd));

        strfcat(snd, "&i[0]=", sizeof(snd));
        strfcat(snd, utm, sizeof(snd));

        strfcat(snd, "&o[0]=P&r[0]=&l[0]=", sizeof(snd));
        strfcat(snd, sttime, sizeof(snd));

        strfcat(snd, "&b[0]=", sizeof(snd));
        strfcat(snd, scrob_urlenc(album), sizeof(snd));

        strfcat(snd, "&n[0]=", sizeof(snd));
        strfcat(snd, scrob_urlenc(track), sizeof(snd));

        strfcat(snd, "&m[0]=", sizeof(snd));

        scrob_send_submission(sock, snd, scrob_submiturl);
}

/**
 * @brief Send now playing to scrobbler.
 *
 * @param artist
 * @param title
 * @param album
 * @param track
 * @param ttime
 */
static void scrob_nowplay(int sock, const char *artist, const char *title,
                          const char *album, const char *track, const int ttime)
{
        char snd[1024];
        char sttime[32];

        DEBUGF(("Sending now playing (%s - %s) to (%s)!\n",
                artist, title, scrob_nowplayurl));

        uint_to_str(sttime, ttime, sizeof(sttime));

        /* Assemble request string. */
        strfcpy(snd, "s=", sizeof(snd));
        strfcat(snd, scrob_sessionid, sizeof(snd));

        strfcat(snd, "&a=", sizeof(snd));
        strfcat(snd, scrob_urlenc(artist), sizeof(snd));

        strfcat(snd, "&t=", sizeof(snd));
        strfcat(snd, scrob_urlenc(title), sizeof(snd));

        strfcat(snd, "&b=", sizeof(snd));
        strfcat(snd, scrob_urlenc(album), sizeof(snd));

        strfcat(snd, "&l=", sizeof(snd));
        strfcat(snd, sttime, sizeof(snd));

        strfcat(snd, "&n=", sizeof(snd));
        strfcat(snd, scrob_urlenc(track), sizeof(snd));

        strfcat(snd, "&m=", sizeof(snd));

        scrob_send_submission(sock, snd, scrob_nowplayurl);
}

/**
 * @brief Send submission/nowplaying to server.
 *
 * @param snd Send string
 * @param url URL to post to
 */
static void scrob_send_submission(int sock, const char *snd, const char *url)
{
        char buf[1024];
        char *line;
        unsigned int i;
        unsigned int len;
        int ok; /* bool */
        int n;

        ok = 0;
        len = strlen(snd);
        
        for (i = 0; i < SCROB_RETRIES; i++) {
                sendcrlf(sock, "POST %s HTTP/1.1", url);
                sendcrlf(sock, "Host: %s", scrob_host);
                sendcrlf(sock, "User-Agent: %s/%s", SCROB_CLIENT, SCROB_VERSION);
                sendcrlf(sock, "Content-Type: application/x-www-form-urlencoded");
                sendcrlf(sock, "Content-Length: %d", len);
                sendx(sock, "\r\n");
                sendx(sock, snd);

                n = recv(sock, buf, sizeof(buf), 0);
                if (n <= 0)
                        return;

                buf[n] = '\0';

                for (line = strtok(buf, "\r\n");
                     line; line = strtok(NULL, "\r\n")) {
                        if (!strcmp(line, "OK")) {
                                ok = 1;
                                break;
                        }
                }

                if (ok) {
                        DEBUGF(("mpdscrob: Submission successful!\n"));
                        return;
                }

                /* We got something other than OK ;[ */
                close(sock);

                /* Connect to the scrobbler server. */
                sock = scrob_connect();
                if (sock == -1) {
                        fprintf(stderr,
                                "Could not connect to scrobbler server ;[\n");
                        return;
                }

                /* We most likely need to redo the handshake. */
                if ((scrob_handshake(sock) == -1)) {
                        scrob_shaked = 0;
                        return;
                }
        }
}

/**
 * @brief Do a simple url encoding.
 *
 * @param str String to encode
 *
 * @return Encoded string
 */
static char *scrob_urlenc(const char *str)
{

        char *n;
        int pos;
        int len;
        char buf[4];

        /* This is technically the safest size to make this I think ;\ */
        len = strlen(str) * 3 + 1;
        n = m_malloc(len);
        n[0] = '\0';

        pos = 0;

        for (; *str; str++) {
                switch (*str) {
                case ' ':
                        n[pos] = '+';
                        pos++;
                        break;
                case '&':
                case '=':
                case '%':
                case '+':
                        sprintf(buf, "%%%02x", *str);
                        strcat(n + pos, buf);
                        pos += 3;
                        break;
                default:
                        n[pos] = *str;
                        pos++;
                        break;
                }
        }

        n[pos] = '\0';
        return n;
}

/**
 * @brief Create socket and connect to scrobbler server.
 *
 * @return Socket
 */
static int scrob_connect(void)
{
        int sock;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "Could not create scrob socket: %s\n",
                        strerror(errno));
                return -1;
        }

        if ((connect(sock, (struct sockaddr *) &server, sizeof(server)) == -1)) {
                fprintf(stderr, "Could not connect to scrob socket: %s\n",
                        strerror(errno));
                close(sock);
                return -1;
        }

        return sock;
}
