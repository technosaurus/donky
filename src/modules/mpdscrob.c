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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../cfg.h"
#include "../mem.h"
#include "../util.h"
#include "../std/stdbool.h"
#include "../std/string.h"
#include "mpdscrob.h"
#include "extra/md5.h"

#define SCROB_CLIENT "dms"
#define SCROB_VERSION "1.0"

/* Reminder: try to find a portable strftime alternative for getting the UNIX
 * timestamp.  I'm sure there is something I just do not know of it and haven't
 * found it yet!  I'd like this module to be totally ANSI compliant. */

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
bool scrob_enabled;
static char *scrob_host;
static int scrob_port;
static char *scrob_user;
static char *scrob_pass;
static struct sockaddr_in server;
static struct hostent *hptr;
static bool scrob_shaked;
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

/**
 * @brief Called with mpd's init_settings method.
 */
void scrob_init(void)
{
        last_artist[0] = '\0';
        last_title[0] = '\0';
        last_etime = 0;
        last_ttime = 0;

        scrob_shaked = false;
        scrob_sessionid[0] = '\0';

        scrob_host = get_char_key("mpd", "scrob_host", NULL);
        scrob_port = get_int_key("mpd", "scrob_port", 80);
        scrob_enabled = get_bool_key("mpd", "scrob_enabled", false);
        scrob_user = get_char_key("mpd", "scrob_user", NULL);
        scrob_pass = get_char_key("mpd", "scrob_pass", NULL);

        if (scrob_host == NULL)
                scrob_enabled = false;

        if (scrob_enabled) {
                if ((hptr = gethostbyname(scrob_host)) == NULL) {
                        fprintf(stderr,
                                "Could not gethostbyname(%s)\n", scrob_host);
                        scrob_enabled = false;
                        return;
                }

                memcpy(&server.sin_addr, hptr->h_addr_list[0], hptr->h_length);
                server.sin_family = AF_INET;
                server.sin_port = htons((short) scrob_port);
        }
}

/**
 * @brief Called when the mpd module is destroyed.
 */
void scrob_die(void)
{
        free(scrob_host);
        free(scrob_user);
        free(scrob_pass);
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
                printf("Could not connect to scrobbler server ;[\n");
                return;
        }

        /* We haven't done the handshake. */
        if (!scrob_shaked) {
                if ((scrob_handshake(sock) == -1))
                        return;

                scrob_shaked = true;
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

        /* Update all the last_* stuff. */
        strlcpy(last_artist, artist, sizeof(last_artist));
        strlcpy(last_title, title, sizeof(last_title));
        strlcpy(last_album, album, sizeof(last_album));
        strlcpy(last_track, track, sizeof(last_track));
        last_ttime = ttime;

        /* Close this maw faw. */
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
        time_t t;
        struct tm *tmp;
        char utm[96];
        char utm_md5[160];
        int n;
        char buf[1024];
        char *line;

        printf("Handshaking with (%s:%d)!\n", scrob_host, scrob_port);

        t = time(NULL);
        tmp = localtime(&t);

        if (strftime(utm, sizeof(utm) - sizeof(char), "%s", tmp) == 0) {
                printf("mpdscrob: Issues with strftime!\n");
                return -1;
        }

        strlcpy(utm_md5, scrob_pass, sizeof(utm_md5));
        strlcat(utm_md5, utm, sizeof(utm_md5));

        scrob_md5(utm_md5);

        sendcrlf(sock,
                 "GET http://%s/"
                 "?hs=true&p=1.2&c=%s&v=%s&u=%s&t=%s&a=%s",
                 scrob_host,
                 SCROB_CLIENT,
                 SCROB_VERSION,
                 scrob_user,
                 utm,
                 utm_md5);

        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0) {
                printf("Socket disconnected!\n");
                return -1;
        }

        buf[n] = '\0';

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

        strlcpy(scrob_sessionid, line, sizeof(scrob_sessionid));

        /* Now playing URL */
        line = strtok(NULL, "\n");
        if (line == NULL)
                return -1;

        strlcpy(scrob_nowplayurl, line, sizeof(scrob_nowplayurl));

        /* Submit URL */
        line = strtok(NULL, "\n");
        if (line == NULL)
                return -1;

        strlcpy(scrob_submiturl, line, sizeof(scrob_submiturl));

        /* Success. */
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
        char buf[1024];
        int n;
        time_t t;
        struct tm *tmp;
        char utm[96];
        int tries;
        char snd[1024];
        char sttime[32];
        int len;
        char *line;
        bool ok;

        printf("Submitting (%s - %s) to (%s)!\n", artist, title, scrob_submiturl);

        tries = 0;
        ok = false;

        t = time(NULL);
        tmp = localtime(&t);

        if (strftime(utm, sizeof(utm) - sizeof(char), "%s", tmp) == 0) {
                printf("mpdscrob: Issues with strftime!\n");
                return;
        }

        uint_to_str(sttime, ttime, sizeof(sttime));

        /* Assemble request string. */
        strlcpy(snd, "s=", sizeof(snd));
        strlcat(snd, scrob_sessionid, sizeof(snd));

        strlcat(snd, "&a[0]=", sizeof(snd));
        strlcat(snd, scrob_urlenc(artist), sizeof(snd));

        strlcat(snd, "&t[0]=", sizeof(snd));
        strlcat(snd, scrob_urlenc(title), sizeof(snd));

        strlcat(snd, "&i[0]=", sizeof(snd));
        strlcat(snd, utm, sizeof(snd));

        strlcat(snd, "&o[0]=P&r[0]=&l[0]=", sizeof(snd));
        strlcat(snd, sttime, sizeof(snd));

        strlcat(snd, "&b[0]=", sizeof(snd));
        strlcat(snd, scrob_urlenc(album), sizeof(snd));

        strlcat(snd, "&n[0]=", sizeof(snd));
        strlcat(snd, scrob_urlenc(track), sizeof(snd));

        strlcat(snd, "&m[0]=", sizeof(snd));

        len = strlen(snd);

SUBMITPOST:
        tries++;
        sendcrlf(sock, "POST %s HTTP/1.1", scrob_submiturl);
        sendcrlf(sock, "Host: %s", scrob_host);
        sendcrlf(sock, "User-Agent: donkympd/2009.5");
        sendcrlf(sock, "Content-Type: application/x-www-form-urlencoded");
        sendcrlf(sock, "Content-Length: %d", len);
        sendx(sock, "\r\n");
        sendx(sock, snd);

        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0)
                return;

        buf[n] = '\0';
        chomp(buf);

        for (line = strtok(buf, "\r\n"); line; line = strtok(NULL, "\r\n")) {
                if (!strcmp(line, "OK")) {
                        ok = true;
                        break;
                }
        }

        /* We got something other than OK ;[ */
        if (!ok) {
                close(sock);

                /* Connect to the scrobbler server. */
                sock = scrob_connect();
                if (sock == -1) {
                        printf("Could not connect to scrobbler server ;[\n");
                        return;
                }

                /* We most likely need to redo the handshake. */
                if ((scrob_handshake(sock) == -1)) {
                        /* TODO: Add to cache.  If server connectivity is ever
                         * problematic, or our own internet connection goes
                         * down, it would be nice to cache what we are playing,
                         * and send it out later when network connectivity
                         * is back. */

                        scrob_shaked = false;
                        return;
                }

                if (tries < 3)
                        goto SUBMITPOST;
        }
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
        char buf[1024];
        int n;
        int tries;
        char snd[1024];
        char sttime[32];
        int len;
        char *line;
        bool ok;

        printf("Sending now playing (%s - %s) to (%s)!\n",
               artist, title, scrob_nowplayurl);

        tries = 0;
        ok = false;

        uint_to_str(sttime, ttime, sizeof(sttime));

        /* Assemble request string. */
        strlcpy(snd, "s=", sizeof(snd));
        strlcat(snd, scrob_sessionid, sizeof(snd));

        strlcat(snd, "&a=", sizeof(snd));
        strlcat(snd, scrob_urlenc(artist), sizeof(snd));

        strlcat(snd, "&t=", sizeof(snd));
        strlcat(snd, scrob_urlenc(title), sizeof(snd));

        strlcat(snd, "&b=", sizeof(snd));
        strlcat(snd, scrob_urlenc(album), sizeof(snd));

        strlcat(snd, "&l=", sizeof(snd));
        strlcat(snd, sttime, sizeof(snd));

        strlcat(snd, "&n=", sizeof(snd));
        strlcat(snd, scrob_urlenc(track), sizeof(snd));

        strlcat(snd, "&m=", sizeof(snd));

        len = strlen(snd);

NOWPLAYPOST:
        tries++;
        sendcrlf(sock, "POST %s HTTP/1.1", scrob_nowplayurl);
        sendcrlf(sock, "Host: %s", scrob_host);
        sendcrlf(sock, "User-Agent: donkympd/2009.5");
        sendcrlf(sock, "Content-Type: application/x-www-form-urlencoded");
        sendcrlf(sock, "Content-Length: %d", len);
        sendx(sock, "\r\n");
        sendx(sock, snd);

        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0)
                return;

        buf[n] = '\0';
        chomp(buf);

        for (line = strtok(buf, "\r\n"); line; line = strtok(NULL, "\r\n")) {
                if (!strcmp(line, "OK")) {
                        ok = true;
                        break;
                }
        }

        /* We got something other than OK ;[ */
        if (!ok) {
                close(sock);

                /* Connect to the scrobbler server. */
                sock = scrob_connect();
                if (sock == -1) {
                        printf("Could not connect to scrobbler server ;[\n");
                        return;
                }

                /* We most likely need to redo the handshake. */
                if ((scrob_handshake(sock) == -1)) {
                        scrob_shaked = false;
                        return;
                }

                if (tries < 3)
                        goto NOWPLAYPOST;
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
