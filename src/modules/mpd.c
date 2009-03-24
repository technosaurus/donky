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

/* Globals. */
int mpd_sock = -1;
FILE *sockin, *sockout;

struct mpd_info {
        /* currentsong */
        char file[256];
        char artist[36];
        char title[36];
        char album[36];
        char track[4];
        char date[8];
        char genre[36];

        /* status */
        char volume[4];
        char repeat[4];
        char random[4];
        char playlist[12];
        char playlistlength[12];
        char xfade[4];
        char state[16];
        char song[4];
        char time[8];
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
        module_var_add(module_name, "mpd_time", "get_timez", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_bitrate", "get_bitrate", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_audio", "get_audio", 10.0, VARIABLE_STR);

        /* Add cron job. */
        module_var_cron_add(module_name, "mpd_cron", "run_cron", 1.0, VARIABLE_CRON);
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
        
}

/**
 * @brief This is run every cron timeout.
 */
void run_cron(void)
{
        printf("Running mpd_cron!\n");
        
        if (start_connection()) {
                pop_currentsong();
                pop_status();
                close(mpd_sock);
                fflush(sockin);
                fflush(sockout);
                fclose(sockin);
                fclose(sockout);
        }
}

void pop_status(void)
{
        char buffer[256];
        char *str;
        
        /* currentsong */
        fprintf(sockout, "status\r\n");
        fflush(sockout);

        while (fgets(buffer, sizeof(buffer), sockin)) {
                str = NULL;

                if (!strcmp(buffer, "OK\n"))
                        break;
                else if (sscanf(buffer, "volume: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.volume, sizeof(mpdinfo.volume),
                                 "%s", str);
                else if (sscanf(buffer, "repeat: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.repeat, sizeof(mpdinfo.repeat),
                                 "%s", str);
                else if (sscanf(buffer, "random: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.random, sizeof(mpdinfo.random),
                                 "%s", str);
                else if (sscanf(buffer, "playlist: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.playlist, sizeof(mpdinfo.playlist),
                                 "%s", str);
                else if (sscanf(buffer, "playlistlength: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.playlistlength, sizeof(mpdinfo.playlistlength),
                                 "%s", str);
                else if (sscanf(buffer, "xfade: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.xfade, sizeof(mpdinfo.xfade),
                                 "%s", str);
                else if (sscanf(buffer, "state: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.state, sizeof(mpdinfo.state),
                                 "%s", str);
                else if (sscanf(buffer, "song: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.song, sizeof(mpdinfo.song),
                                 "%s", str);
                else if (sscanf(buffer, "time: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.time, sizeof(mpdinfo.time),
                                 "%s", str);
                else if (sscanf(buffer, "bitrate: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.bitrate, sizeof(mpdinfo.bitrate),
                                 "%s", str);
                else if (sscanf(buffer, "audio: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.audio, sizeof(mpdinfo.audio),
                                 "%s", str);

                freeif(str);
        }
}

void pop_currentsong(void)
{
        char buffer[256];
        char *str;

        printf("In current song!\n");
        
        /* currentsong */
        fprintf(sockout, "currentsong\r\n");
        fflush(sockout);

        while (fgets(buffer, sizeof(buffer), sockin)) {
                str = NULL;

                if (!strcmp(buffer, "OK\n"))
                        break;
                else if (sscanf(buffer, "file: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.file, sizeof(mpdinfo.file),
                                 "%s", str);
                else if (sscanf(buffer, "Artist: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.artist, sizeof(mpdinfo.artist),
                                 "%s", str);
                else if (sscanf(buffer, "Title: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.title, sizeof(mpdinfo.title),
                                 "%s", str);
                else if (sscanf(buffer, "Album: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.album, sizeof(mpdinfo.album),
                                 "%s", str);
                else if (sscanf(buffer, "Track: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.track, sizeof(mpdinfo.track),
                                 "%s", str);
                else if (sscanf(buffer, "Date: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.date, sizeof(mpdinfo.date),
                                 "%s", str);
                else if (sscanf(buffer, "Genre: %a[^\r\n]", &str) == 1)
                        snprintf(mpdinfo.genre, sizeof(mpdinfo.genre),
                                 "%s", str);
                                 
                freeif(str);
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
char *get_timez(char *args) { return m_strdup(mpdinfo.time); }
char *get_bitrate(char *args) { return m_strdup(mpdinfo.bitrate); }
char *get_audio(char *args) { return m_strdup(mpdinfo.audio); }

/**
 * @brief Connect to MPD host.
 */
int start_connection(void)
{
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        char *mpd_host;
        char *mpd_port;
        int bytes;
        char data[32];
        int s;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        if ((mpd_host = get_char_key("mpd", "host")) == NULL)
                mpd_host = strdup("localhost");

        if ((mpd_port = get_char_key("mpd", "port")) == NULL)
                mpd_port = strdup("6600");

        if ((s = getaddrinfo(mpd_host, mpd_port, &hints, &result)) != 0) {
                fprintf(stderr,
                        "mpd: Could not get host [%s] by name: %s",
                        mpd_host,
                        gai_strerror(s));
                freeif(mpd_host);
                freeif(mpd_port);
                return 0;
        }

        printf("Connecting to [%s]:[%s]\n", mpd_host, mpd_port);
        free(mpd_host);
        free(mpd_port);

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

        freeaddrinfo(result);
        
        /* Wait for OK */
        bytes = recv(mpd_sock, data, 32, 0);
        data[bytes] = '\0';

        if (strstr(data, "OK MPD")) {
                printf("GOT OK MPD\n");
                sockin = fdopen(mpd_sock, "r");
                sockout = fdopen(mpd_sock, "w");
                return 1;
        }

        return 0;
}
