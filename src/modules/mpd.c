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

#include "../c99.h"
#include "../config.h"
#include "../mem.h"
#include "../module.h"
#include "../util.h"

char module_name[] = "mpd";

/* My function prototypes. */
int start_connection(void);
void mpd_free_everythang(void);
void pop_currentsong(void);
void pop_status(void);
void init_settings(void);

/* Globals. */
int mpd_sock = -1;
FILE *sockin, *sockout;
char *mpd_host;
char *mpd_port;
struct addrinfo hints;
struct addrinfo *result;
struct addrinfo *rp;
int initialized = 0;

struct mpd_info {
        /* currentsong */
        char *file;
        char *artist;
        char *title;
        char *album;
        char *track;
        char *date;
        char *genre;

        /* status */
        char *volume;
        char *repeat;
        char *random;
        char *playlist;
        char *playlistlength;
        char *xfade;
        char *state;
        char *song;
        int etime;
        int ttime;
        char *bitrate;
        char *audio;
};

struct mpd_info mpdinfo = {
        NULL,
        NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL,    0,    0, NULL, NULL /* Pretty huh! */
}; /* YES! IT'S LIKE A PYRAMID! sideways. */

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

        free(mpd_host);
        free(mpd_port);
        freeaddrinfo(result);
}

/**
 * @brief Free a bunch of CRAP!
 */
void mpd_free_everything(void)
{
        freenull(mpdinfo.file);
        freenull(mpdinfo.artist);
        freenull(mpdinfo.title);
        freenull(mpdinfo.album);
        freenull(mpdinfo.track);
        freenull(mpdinfo.date);
        freenull(mpdinfo.genre);
        freenull(mpdinfo.volume);
        freenull(mpdinfo.repeat);
        freenull(mpdinfo.random);
        freenull(mpdinfo.playlist);
        freenull(mpdinfo.playlistlength);
        freenull(mpdinfo.xfade);
        freenull(mpdinfo.state);
        freenull(mpdinfo.song);
        freenull(mpdinfo.bitrate);
        freenull(mpdinfo.audio);
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

        mpd_free_everything();
        pop_currentsong();
        pop_status();
}

/**
 * @brief Populate the status variables.
 */
void pop_status(void)
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
                else if (sscanf(buffer, "volume: %m[^\n]", &mpdinfo.volume) == 1) { }
                else if (sscanf(buffer, "repeat: %m[^\n]", &mpdinfo.repeat) == 1) { }
                else if (sscanf(buffer, "random: %m[^\n]", &mpdinfo.random) == 1) { }
                else if (sscanf(buffer, "playlist: %m[^\n]", &mpdinfo.playlist) == 1) { }
                else if (sscanf(buffer, "playlistlength: %m[^\n]", &mpdinfo.playlistlength) == 1) { }
                else if (sscanf(buffer, "xfade: %m[^\n]", &mpdinfo.xfade) == 1) { }
                else if (sscanf(buffer, "state: %m[^\n]", &mpdinfo.state) == 1) { }
                else if (sscanf(buffer, "song: %m[^\n]", &mpdinfo.song) == 1) { }
                else if (sscanf(buffer, "time: %d:%d", &mpdinfo.etime, &mpdinfo.ttime) == 2) { }
                else if (sscanf(buffer, "bitrate: %m[^\n]", &mpdinfo.bitrate) == 1) { }
                else if (sscanf(buffer, "audio: %m[^\n]", &mpdinfo.audio) == 1) { }
        }
}

/**
 * @brief Populate the currentsong variables.
 */
void pop_currentsong(void)
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
                else if (sscanf(buffer, "file: %m[^\n]", &mpdinfo.file) == 1) { }
                else if (sscanf(buffer, "Artist: %m[^\n]", &mpdinfo.artist) == 1) { }
                else if (sscanf(buffer, "Title: %m[^\n]", &mpdinfo.title) == 1) { }
                else if (sscanf(buffer, "Album: %m[^\n]", &mpdinfo.album) == 1) { }
                else if (sscanf(buffer, "Track: %m[^\n]", &mpdinfo.track) == 1) { }
                else if (sscanf(buffer, "Date: %m[^\n]", &mpdinfo.date) == 1) { }
                else if (sscanf(buffer, "Genre: %m[^\n]", &mpdinfo.genre) == 1) { }
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

char *get_file(char *args) { return (mpdinfo.file) ? mpdinfo.file : "\0"; }
char *get_artist(char *args) { return (mpdinfo.artist) ? mpdinfo.artist : "\0"; }
char *get_title(char *args) { return (mpdinfo.title) ? mpdinfo.title : "\0"; }
char *get_album(char *args) { return (mpdinfo.album) ? mpdinfo.album : "\0"; }
char *get_track(char *args) { return (mpdinfo.track) ? mpdinfo.track : "\0"; }
char *get_date(char *args) { return (mpdinfo.date) ? mpdinfo.date : "\0"; }
char *get_genre(char *args) { return (mpdinfo.genre) ? mpdinfo.genre : "\0"; }
char *get_volume(char *args) { return (mpdinfo.volume) ? mpdinfo.volume : "0"; }
char *get_repeat(char *args) { return (mpdinfo.repeat) ? mpdinfo.repeat : "\0"; }
char *get_random(char *args) { return (mpdinfo.random) ? mpdinfo.random : "\0"; }
char *get_playlist(char *args) { return (mpdinfo.playlist) ? mpdinfo.playlist : "\0"; }
char *get_playlistlength(char *args) { return (mpdinfo.playlistlength) ? mpdinfo.playlistlength : "\0"; }
char *get_xfade(char *args) { return (mpdinfo.xfade) ? mpdinfo.xfade : "\0"; }
char *get_song(char *args) { return (mpdinfo.song) ? mpdinfo.song : "\0"; }
char *get_bitrate(char *args) { return (mpdinfo.bitrate) ? mpdinfo.bitrate : "0"; }
char *get_audio(char *args) { return (mpdinfo.audio) ? mpdinfo.audio : "\0"; }

char *get_elapsed_time(char *args) {
        char ret[8];
        
        if (mpdinfo.etime) {
                snprintf(ret, sizeof(ret),
                         "%.2d:%.2d",
                         mpdinfo.etime / 60, mpdinfo.etime % 60);
                return m_freelater(ret);
        }

        return "00:00";
}

char *get_total_time(char *args) {
        char ret[8];
        
        if (mpdinfo.ttime) {
                snprintf(ret, sizeof(ret),
                         "%.2d:%.2d",
                         mpdinfo.ttime / 60, mpdinfo.ttime % 60);
                return m_freelater(ret);
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
void init_settings(void)
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
                return;
        }
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
