/*
 * This file is part of donky.
 *
 * donky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * donky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.
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

#include "develop.h"
#include "../util.h"
#include "../config.h"
#include "../mem.h"

char module_name[] = "mpd";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes. */
int start_connection(void);
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

int disable_mpd_file;
int disable_mpd_artist;
int disable_mpd_title;
int disable_mpd_album;
int disable_mpd_track;
int disable_mpd_date;
int disable_mpd_genre;
int disable_mpd_volume;
int disable_mpd_repeat;
int disable_mpd_random;
int disable_mpd_playlist;
int disable_mpd_playlistlength;
int disable_mpd_xfade;
int disable_mpd_state;
int disable_mpd_song;
int disable_mpd_etime;
int disable_mpd_ttime;
int disable_mpd_bitrate;
int disable_mpd_audio;

struct mpd_info {
        /* currentsong */
        char file[256];
        char artist[64];
        char title[64];
        char album[64];
        char track[8];
        char date[8];
        char genre[32];

        /* status */
        char volume[4];
        char repeat[4];
        char random[4];
        char playlist[8];
        char playlistlength[8];
        char xfade[4];
        char state[12];
        char song[8];
        char etime[8];
        char ttime[8];
        char bitrate[8];
        char audio[16];
} mpdinfo;

/**
 * @brief This is run on module initialization.
 */
int module_init(void)
{
        module_var_add(module_name, "mpd_file", "get_file", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_artist", "get_artist", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_title", "get_title", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_album", "get_album", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_track", "get_track", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_date", "get_date", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_genre", "get_genre", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_volume", "get_volume", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_repeat", "get_repeat", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_random", "get_random", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_playlist", "get_playlist", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_playlistlength", "get_playlistlength", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_xfade", "get_xfade", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_state", "get_state", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_song", "get_song", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_etime", "get_elapsed_time", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_ttime", "get_total_time", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_bitrate", "get_bitrate", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_audio", "get_audio", 10.0, VARIABLE_STR);

        module_var_add(module_name, "mpd_volume_bar", "get_volume_bar", 10.0, VARIABLE_BAR);

        /* Add cron job. */
        module_var_cron_add(module_name, "mpd_cron", "run_cron", 1.0, VARIABLE_CRON);        
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
        /* Disconnect from mpd. */
        if (mpd_sock != -1) {
                fprintf(sockout, "close\r\n");
                fflush(sockout);
                fflush(sockin);
                fclose(sockout);
                fclose(sockin);
                close(mpd_sock);
        }
        
        /* Free all my memorah! */
        freeif(mpd_host);
        freeif(mpd_port);
        freeaddrinfo(result);
}

/**
 * @brief This is run every cron timeout.
 */
void run_cron(void)
{
        /* Do some initial shizzy. */
        if (!initialized) {
                init_settings();
                initialized = 1;
        }

        if (mpd_sock == -1)
                start_connection();
        
        pop_currentsong();
        pop_status();
}

/**
 * @brief Populate the status variables.
 */
void pop_status(void)
{
        char buffer[128];

        /* will be used for \n search & removal */
        char *p = NULL;

        /* will be used for etime and ttime */
        int elapsed;
        int total;
        int i;

        size_t n = sizeof(char);
        
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
                /* "volume" is 6 characters long.
                 * (buffer + 8) is where the actual value of "volume"
                 * begins in the string. This offset of 2 generalizes
                 * for every other variable minus checking
                 * playlist vs playlistlength and the etime and ttime
                 * variables for which we use sscanf instead. */
                else if (!disable_mpd_volume && !strncmp(buffer, "volume", 6))
                        p = strncpy(mpdinfo.volume,
                                    (buffer + 8),
                                    sizeof(mpdinfo.volume) - n);
                else if (!disable_mpd_repeat && !strncmp(buffer, "repeat", 6))
                        p = strncpy(mpdinfo.repeat,
                                    (buffer + 8),
                                    sizeof(mpdinfo.repeat) - n);
                else if (!disable_mpd_random && !strncmp(buffer, "random", 6))
                        p = strncpy(mpdinfo.random,
                                    (buffer + 8),
                                    sizeof(mpdinfo.random) - n);
                /* We must include the ":" here, else this would match
                 * both "playlist" and "playlistlength" */
                else if (!disable_mpd_playlist && !strncmp(buffer, "playlist:", 9))
                        p = strncpy(mpdinfo.playlist,
                                    /* ...so the offset is only +1 */
                                    (buffer + 10),
                                    sizeof(mpdinfo.playlist) - n);
                else if (!disable_mpd_playlistlength && !strncmp(buffer, "playlistlength", 14))
                        p = strncpy(mpdinfo.playlistlength,
                                    (buffer + 16),
                                    sizeof(mpdinfo.playlistlength) - n);
                else if (!disable_mpd_xfade && !strncmp(buffer, "xfade", 5))
                        p = strncpy(mpdinfo.xfade,
                                    (buffer + 7),
                                    sizeof(mpdinfo.xfade) - n);
                else if (!disable_mpd_state && !strncmp(buffer, "state", 5)) {
                        /* MPD state is one of:
                         * "play", "pause", or "stop". 
                         * These are not very readable, so we
                         * instead store the state as:
                         * "Playing", "Paused", or "Stopped". */
                        if (!strncmp((buffer + 7), "pl", 2))
                                strncpy(mpdinfo.state,
                                        "Playing",
                                        sizeof(mpdinfo.state) - n);
                        else if (!strncmp((buffer + 7), "pa", 2))
                                strncpy(mpdinfo.state,
                                        "Paused",
                                        sizeof(mpdinfo.state) - n);
                        else if (!strncmp((buffer + 7), "st", 2))
                                strncpy(mpdinfo.state,
                                        "Stopped",
                                        sizeof(mpdinfo.state) - n);
                }
                else if (!disable_mpd_song && !strncmp(buffer, "song", 4))
                        p = strncpy(mpdinfo.song,
                                    (buffer + 6),
                                    sizeof(mpdinfo.song) - n);
                /* MPD gives track elapsed time and total time in an
                 * elapsed:total format, in seconds. This extracts
                 * those numbers and turns them into 2 separate
                 * elements in MM:SS format. */
                else if (!(disable_mpd_ttime && disable_mpd_etime) &&
                         sscanf(buffer, "time: %d:%d", &elapsed, &total) == 2) {
                        snprintf(mpdinfo.etime,
                                 sizeof(mpdinfo.etime) - n,
                                 /* %.2d gives us a leading zero */
                                 "%.2d:%.2d",
                                 /* minutes */   /* seconds */
                                 (elapsed / 60), (elapsed % 60));
                        snprintf(mpdinfo.ttime,
                                 sizeof(mpdinfo.ttime) - n,
                                 "%.2d:%.2d",
                                 (total / 60), (total % 60));
                }
                else if (!disable_mpd_bitrate && !strncmp(buffer, "bitrate", 7))
                        p = strncpy(mpdinfo.bitrate,
                                    (buffer + 9),
                                    sizeof(mpdinfo.bitrate) - n);
                else if (!disable_mpd_audio && !strncmp(buffer, "audio", 5))
                        p = strncpy(mpdinfo.audio,
                                    (buffer + 7),
                                    sizeof(mpdinfo.audio) - n);

                /* Remove newline if there is one. */
                if (p) {
                        chomp(p);
                        p = NULL;
                }
        }
}

/**
 * @brief Populate the currentsong variables.
 */
void pop_currentsong(void)
{
        char buffer[128];
        char *p = NULL;
        size_t n = sizeof(char);
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
                else if (!disable_mpd_file && !strncmp(buffer, "file", 4))
                        p = strncpy(mpdinfo.file,
                                    (buffer + 6),
                                    sizeof(mpdinfo.file) - n);
                else if (!disable_mpd_artist && !strncmp(buffer, "Artist", 6))
                        p = strncpy(mpdinfo.artist,
                                    (buffer + 8),
                                    sizeof(mpdinfo.artist) - n);
                else if (!disable_mpd_title && !strncmp(buffer, "Title", 5))
                        p = strncpy(mpdinfo.title,
                                    (buffer + 7),
                                    sizeof(mpdinfo.title) - n);
                else if (!disable_mpd_album && !strncmp(buffer, "Album", 5))
                        p = strncpy(mpdinfo.album,
                                    (buffer + 7),
                                    sizeof(mpdinfo.album) - n);
                else if (!disable_mpd_track && !strncmp(buffer, "Track", 5))
                        p = strncpy(mpdinfo.track,
                                    (buffer + 7),
                                    sizeof(mpdinfo.track) - n);
                else if (!disable_mpd_date && !strncmp(buffer, "Date", 4))
                        p = strncpy(mpdinfo.date,
                                    (buffer + 6),
                                    sizeof(mpdinfo.date) - n);
                else if (!disable_mpd_genre && !strncmp(buffer, "Genre", 5))
                        p = strncpy(mpdinfo.genre,
                                    (buffer + 7),
                                    sizeof(mpdinfo.genre) - n);

                if (p) {
                        chomp(p);
                        p = NULL;
                }
        }
}

char *get_file(char *args) { return m_strdup(mpdinfo.file); }
char *get_artist(char *args) { return m_strdup(mpdinfo.artist); }
char *get_title(char *args) { return m_strdup(mpdinfo.title); }
char *get_album(char *args) { return m_strdup(mpdinfo.album); }
char *get_track(char *args) { return m_strdup(mpdinfo.track); }
char *get_date(char *args) { return m_strdup(mpdinfo.date); }
char *get_genre(char *args) { return m_strdup(mpdinfo.genre); }
char *get_volume(char *args) { return m_strdup(mpdinfo.volume); }
char *get_repeat(char *args) { return m_strdup(mpdinfo.repeat); }
char *get_random(char *args) { return m_strdup(mpdinfo.random); }
char *get_playlist(char *args) { return m_strdup(mpdinfo.playlist); }
char *get_playlistlength(char *args) { return m_strdup(mpdinfo.playlistlength); }
char *get_xfade(char *args) { return m_strdup(mpdinfo.xfade); }
char *get_state(char *args) { return m_strdup(mpdinfo.state); }
char *get_song(char *args) { return m_strdup(mpdinfo.song); }
char *get_bitrate(char *args) { return m_strdup(mpdinfo.bitrate); }
char *get_audio(char *args) { return m_strdup(mpdinfo.audio); }
char *get_elapsed_time(char *args) { return m_strdup(mpdinfo.etime); }
char *get_total_time(char *args) { return m_strdup(mpdinfo.ttime); }
int get_volume_bar(char *args) { return strtol(mpdinfo.volume, NULL, 0); }

/**
 * @brief Initialize some things that we use quite often.
 */
void init_settings(void)
{
        int s;
        
        /* Read configuration settings, if none exist, use some defaults. */
        mpd_host = get_char_key("mpd", "host", "localhost");
        mpd_port = get_char_key("mpd", "port", "6600");

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

        /* Disable settings...
         * I added these so power users (Spewns) can fine tune what they wouldn't
         * like to store. */
        disable_mpd_file = get_bool_key("mpd", "disable_mpd_file", 0);
        disable_mpd_artist = get_bool_key("mpd", "disable_mpd_artist", 0);
        disable_mpd_title = get_bool_key("mpd", "disable_mpd_title", 0);
        disable_mpd_album = get_bool_key("mpd", "disable_mpd_album", 0);
        disable_mpd_track = get_bool_key("mpd", "disable_mpd_track", 0);
        disable_mpd_date = get_bool_key("mpd", "disable_mpd_date", 0);
        disable_mpd_genre = get_bool_key("mpd", "disable_mpd_genre", 0);
        disable_mpd_volume = get_bool_key("mpd", "disable_mpd_volume", 0);
        disable_mpd_repeat = get_bool_key("mpd", "disable_mpd_repeat", 0);
        disable_mpd_random = get_bool_key("mpd", "disable_mpd_random", 0);
        disable_mpd_playlist = get_bool_key("mpd", "disable_mpd_playlist", 0);
        disable_mpd_playlistlength = get_bool_key("mpd", "disable_mpd_playlistlength", 0);
        disable_mpd_xfade = get_bool_key("mpd", "disable_mpd_xfade", 0);
        disable_mpd_state = get_bool_key("mpd", "disable_mpd_state", 0);
        disable_mpd_song = get_bool_key("mpd", "disable_mpd_song", 0);
        disable_mpd_etime = get_bool_key("mpd", "disable_mpd_etime", 0);
        disable_mpd_ttime = get_bool_key("mpd", "disable_mpd_ttime", 0);
        disable_mpd_bitrate = get_bool_key("mpd", "disable_mpd_bitrate", 0);
        disable_mpd_audio = get_bool_key("mpd", "disable_mpd_audio", 0);
}

/**
 * @brief Connect to MPD host.
 */
int start_connection(void)
{
        int bytes;
        char data[32];
        
        //printf("Connecting to [%s]:[%s]\n", mpd_host, mpd_port);

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
