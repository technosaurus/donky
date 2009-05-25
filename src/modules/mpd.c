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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include "../cfg.h"
#include "../mem.h"
#include "../module.h"
#include "../util.h"

char module_name[] = "mpd";

/* My function prototypes. */
static int start_connection(void);
static void mpd_free_everythang(void);
static void pop_currentsong(void);
static void pop_status(void);
static void init_settings(void);

/* Globals. */
int mpd_sock = -1;
FILE *sockin, *sockout;
char *mpd_host = NULL;
char *mpd_port = NULL;
struct addrinfo hints;
struct addrinfo *result = NULL;
struct addrinfo *rp;
int initialized = 0;

struct mpd_info {
        /* currentsong */
        char file[256];
        char artist[128];
        char title[128];
        char album[128];
        char track[16];
        char date[16];
        char genre[64];

        /* status */
        char volume[8];
        char repeat[8];
        char random[8];
        char playlist[16];
        char playlistlength[16];
        char xfade[8];
        char state[8];
        char song[8];
        int etime;
        int ttime;
        char bitrate[16];
        char audio[16];
} mpdinfo;

/**
 * @brief This is run on module initialization.
 */
void module_init(const struct module *mod)
{
        module_var_add(mod, "mpd_file", "get_file", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_artist", "get_artist", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_title", "get_title", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_album", "get_album", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_track", "get_track", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_date", "get_date", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_genre", "get_genre", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_volume", "get_volume", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_repeat", "get_repeat", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_random", "get_random", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_playlist", "get_playlist", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_playlistlength", "get_playlistlength", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_xfade", "get_xfade", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_state", "get_state", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_song", "get_song", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_etime", "get_elapsed_time", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_ttime", "get_total_time", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_bitrate", "get_bitrate", 10.0, VARIABLE_STR);
        module_var_add(mod, "mpd_audio", "get_audio", 10.0, VARIABLE_STR);

        module_var_add(mod, "mpd_volume_bar", "get_volume_bar", 10.0, VARIABLE_BAR);

        module_var_add(mod, "mpd_cron", "run_cron", 1.0, VARIABLE_CRON);
}

/**
 * @brief This is run when the module is about to be unloaded.
 *        Disconnect from mpd and free shiz.
 */
void module_destroy(void)
{
        if (mpd_sock != -1) {
                fprintf(sockout, "close\r\n");
                fflush(sockout);
                fflush(sockin);
                fclose(sockout);
                fclose(sockin);
                close(mpd_sock);
        }

        if (result != NULL)
                freeaddrinfo(result);
}

/**
 * @brief Free a bunch of CRAP!
 */
static void mpd_free_everythang(void)
{
        mpdinfo.file[0] = '\0';
        mpdinfo.artist[0] = '\0';
        mpdinfo.title[0] = '\0';
        mpdinfo.album[0] = '\0';
        mpdinfo.track[0] = '\0';
        mpdinfo.date[0] = '\0';
        mpdinfo.genre[0] = '\0';
        mpdinfo.volume[0] = '\0';
        mpdinfo.repeat[0] = '\0';
        mpdinfo.random[0] = '\0';
        mpdinfo.playlist[0] = '\0';
        mpdinfo.playlistlength[0] = '\0';
        mpdinfo.xfade[0] = '\0';
        mpdinfo.state[0] = '\0';
        mpdinfo.song[0] = '\0';
        mpdinfo.bitrate[0] = '\0';
        mpdinfo.audio[0] = '\0';
        mpdinfo.ttime = 0;
        mpdinfo.etime = 0;
}

/**
 * @brief This is run every cron timeout.
 *        Initialize settings if we haven't, connect if we aren't connected,
 *        and gather MPD status and current song information.
 */
void run_cron(void)
{
        if (!initialized) {
                init_settings();
                initialized = 1;
        }

        if (mpd_sock == -1)
                start_connection();

        mpd_free_everythang();
        pop_currentsong();
        pop_status();
}

/**
 * @brief Populate the status variables.
 */
static void pop_status(void)
{
        char buffer[128];
        int i;

        /* currentsong */
        for (i = 0; i < 3; i++) {
                if (fprintf(sockout, "status\r\n") == -1)
                        start_connection();
                else
                        break;
        }

        fflush(sockout);

        while (fgets(buffer, sizeof(buffer), sockin)) {
                if (!strcmp(buffer, "OK\n"))
                        break;
                else if (sscanf(buffer, "volume: %7[^\n]", mpdinfo.volume) == 1) { }
                else if (sscanf(buffer, "repeat: %7[^\n]", mpdinfo.repeat) == 1) { }
                else if (sscanf(buffer, "random: %7[^\n]", mpdinfo.random) == 1) { }
                else if (sscanf(buffer, "playlist: %15[^\n]", mpdinfo.playlist) == 1) { }
                else if (sscanf(buffer, "playlistlength: %15[^\n]", mpdinfo.playlistlength) == 1) { }
                else if (sscanf(buffer, "xfade: %7[^\n]", mpdinfo.xfade) == 1) { }
                else if (sscanf(buffer, "state: %7[^\n]", mpdinfo.state) == 1) { }
                else if (sscanf(buffer, "song: %7[^\n]", mpdinfo.song) == 1) { }
                else if (sscanf(buffer, "time: %d:%d", &mpdinfo.etime, &mpdinfo.ttime) == 2) { }
                else if (sscanf(buffer, "bitrate: %15[^\n]", mpdinfo.bitrate) == 1) { }
                else if (sscanf(buffer, "audio: %15[^\n]", mpdinfo.audio) == 1) { }
        }
}

/**
 * @brief Populate the currentsong variables.
 */
static void pop_currentsong(void)
{
        char buffer[128];
        int i;

        /* currentsong */
        for (i = 0; i < 3; i++) {
                if (fprintf(sockout, "currentsong\r\n") == -1)
                        start_connection();
                else
                        break;
        }

        fflush(sockout);

        while (fgets(buffer, sizeof(buffer), sockin)) {
                if (!strcmp(buffer, "OK\n"))
                        break;
                else if (sscanf(buffer, "file: %255[^\n]", mpdinfo.file) == 1) { }
                else if (sscanf(buffer, "Artist: %127[^\n]", mpdinfo.artist) == 1) { }
                else if (sscanf(buffer, "Title: %127[^\n]", mpdinfo.title) == 1) { }
                else if (sscanf(buffer, "Album: %127[^\n]", mpdinfo.album) == 1) { }
                else if (sscanf(buffer, "Track: %15[^\n]", mpdinfo.track) == 1) { }
                else if (sscanf(buffer, "Date: %15[^\n]", mpdinfo.date) == 1) { }
                else if (sscanf(buffer, "Genre: %63[^\n]", mpdinfo.genre) == 1) { }
        }
}

/* From here down are merely functions that return the gathered information */
char *get_state(char *args) {
        if (mpdinfo.state) {
                if (!strncmp(mpdinfo.state, "stop", 1))
                        return "Stopped";
                else if (!strncmp(mpdinfo.state, "play", 2))
                        return "Playing";
                else if (!strncmp(mpdinfo.state, "pause", 2))
                        return "Paused";
        }

        return "MPD";
}

char *get_file(char *args) { return mpdinfo.file; }
char *get_artist(char *args) { return mpdinfo.artist; }
char *get_title(char *args) { return mpdinfo.title; }
char *get_album(char *args) { return mpdinfo.album; }
char *get_track(char *args) { return mpdinfo.track; }
char *get_date(char *args) { return mpdinfo.date; }
char *get_genre(char *args) { return mpdinfo.genre; }
char *get_volume(char *args) { return mpdinfo.volume; }
char *get_repeat(char *args) { return mpdinfo.repeat; }
char *get_random(char *args) { return mpdinfo.random; }
char *get_playlist(char *args) { return mpdinfo.playlist; }
char *get_playlistlength(char *args) { return mpdinfo.playlistlength; }
char *get_xfade(char *args) { return mpdinfo.xfade; }
char *get_song(char *args) { return mpdinfo.song; }
char *get_bitrate(char *args) { return mpdinfo.bitrate; }
char *get_audio(char *args) { return mpdinfo.audio; }

char *get_elapsed_time(char *args) {
        char ret[8];
        
        if (mpdinfo.etime) {
                snprintf(ret, sizeof(ret),
                         "%.2d:%.2d",
                         mpdinfo.etime / 60, mpdinfo.etime % 60);
                return m_strdup(ret);
        }

        return "00:00";
}

char *get_total_time(char *args) {
        char ret[8];
        
        if (mpdinfo.ttime) {
                snprintf(ret, sizeof(ret),
                         "%.2d:%.2d",
                         mpdinfo.ttime / 60, mpdinfo.ttime % 60);
                return m_strdup(ret);
        }

        return "00:00";
}

unsigned int get_volume_bar(char *args) {
        if (mpdinfo.volume)
                return strtol(mpdinfo.volume, NULL, 0);
        return 0;
}

/**
 * @brief Initialize some things that we use quite often.
 */
static void init_settings(void)
{
        int s;

        /* Read configuration settings, if none exist, use some defaults. */
        mpd_host = get_char_key(module_name, "host", "localhost");
        mpd_port = get_char_key(module_name, "port", "6600");

        /* Get address information. */
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        if ((s = getaddrinfo(mpd_host, mpd_port, &hints, &result)) != 0) {
                fprintf(stderr,
                        "mpd: Could not get host [%s] by name: %s",
                        mpd_host,
                        gai_strerror(s));
                free(mpd_host);
                free(mpd_port);
                return;
        }

        free(mpd_host);
        free(mpd_port);
}

/**
 * @brief Connect to MPD host.
 */
int start_connection(void)
{
        int bytes;
        char data[32];

        for (rp = result; rp != NULL; rp = rp->ai_next) {
                mpd_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

                if (mpd_sock == -1)
                        continue;

                if (connect(mpd_sock, rp->ai_addr, rp->ai_addrlen) != -1)
                        break;

                close(mpd_sock);
        }

        if (rp == NULL) {
                fprintf(stderr, "Could not connect to mpd server!\n");
                return 0;
        }

        /* Wait for OK */
        bytes = recv(mpd_sock, data, 32, 0);
        data[bytes] = '\0';

        if (strstr(data, "OK MPD")) {
                sockin = fdopen(mpd_sock, "r");
                sockout = fdopen(mpd_sock, "w");
                return 1;
        }

        return 0;
}
