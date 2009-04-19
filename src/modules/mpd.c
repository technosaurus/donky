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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <netdb.h>
//#include <unistd.h>
//#include <errno.h>
#include <libmpd-1.0/libmpd/libmpd.h>
#include <libmpd-1.0/libmpd/libmpdclient.h>
#include <libmpd-1.0/libmpd/libmpd-playlist.h>
#include <libmpd-1.0/libmpd/libmpd-status.h>

#include "develop.h"
#include "../util.h"
#include "../config.h"
#include "../mem.h"

char module_name[] = "mpd";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* Globals. */
char *mpd_host = NULL;
int mpd_port = 0;
char *mpd_pass = NULL;
MpdObj *mpd_conn = NULL;
mpd_Song *mpd_song = NULL;

/**
 * @brief This is run on module initialization.
 */
int module_init(void)
{
        mpd_host = get_char_key("mpd", "host", "localhost");
        mpd_port = get_int_key("mpd", "port", 6600);
        mpd_pass = get_char_key("mpd", "password", NULL);

        if (!mpd_song) mpd_song = mpd_newSong();

        module_var_add(module_name, "mpd_file", "get_file", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_artist", "get_artist", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_title", "get_title", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_album", "get_album", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_track", "get_track", 10.0, VARIABLE_STR);
        module_var_add(module_name, "mpd_name", "get_name", 10.0, VARIABLE_STR);
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
        module_var_cron_add(module_name, "mpd_cron", "run_cron", 10.0, VARIABLE_CRON);        
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
        if (mpd_conn) {
                if (mpd_check_connected(mpd_conn))
                        mpd_disconnect(mpd_conn);

                mpd_free(mpd_conn);
                mpd_conn = NULL;
        }

        if (mpd_host) {
                free(mpd_host);
                mpd_host = NULL;
        }

        if (mpd_port)
                mpd_port = 0;

        if (mpd_pass) {
                free(mpd_pass);
                mpd_pass = NULL;
        }
}

/** 
 * @brief Check if we're connected. If we aren't, try to connect.
 *        If we have a connection, point mpd_song to the current song.
 */
void run_cron(void)
{
        if (!mpd_conn)
                mpd_conn = mpd_new(mpd_host, mpd_port, mpd_pass);

        if (mpd_conn && (!mpd_check_connected(mpd_conn))) {
                if (mpd_connect(mpd_conn) != MPD_OK) {
                        printf("Could not connect to MPD server.\n");
                        mpd_free(mpd_conn);
                        mpd_conn = NULL;
                        if (mpd_song)
                                mpd_song = NULL;
                        return;
                }
        } else {
                mpd_status_update(mpd_conn);
                mpd_song = mpd_playlist_get_current_song(mpd_conn);
        }
}

char *get_state(char *args) {
        if (mpd_conn) {
                switch (mpd_player_get_state(mpd_conn)) {
                case MPD_PLAYER_PAUSE:          return "Paused";
	        case MPD_PLAYER_PLAY:           return "Playing";
	        case MPD_PLAYER_STOP:           return "Stopped";
                case MPD_PLAYER_UNKNOWN:        return "\0";
                default:                        return "\0";
                }
        }

        return "\0";
}

char *get_file(char *args) {
        return (mpd_song && mpd_song->file) ? mpd_song->file : "\0";
}

char *get_artist(char *args) {
        return (mpd_song && mpd_song->artist) ? mpd_song->artist : "\0";
}

char *get_title(char *args) {
        return (mpd_song && mpd_song->title) ? mpd_song->title : "\0";
}

char *get_album(char *args) {
        return (mpd_song && mpd_song->album) ? mpd_song->album : "\0";
}

char *get_track(char *args) {
        return (mpd_song && mpd_song->track) ? mpd_song->track : "\0";
}

char *get_name(char *args) {
        return (mpd_song && mpd_song->name) ? mpd_song->name : "\0";
}

char *get_date(char *args) {
        return (mpd_song && mpd_song->date) ? mpd_song->date : "\0";
}

char *get_genre(char *args) {
        return (mpd_song && mpd_song->genre) ? mpd_song->genre : "\0";
}

char *get_composer(char *args) {
        return (mpd_song && mpd_song->composer) ? mpd_song->composer : "\0";
}

char *get_performer(char *args) {
        return (mpd_song && mpd_song->performer) ? mpd_song->performer : "\0";
}

char *get_disc(char *args) {
        return (mpd_song && mpd_song->disc) ? mpd_song->disc : "\0";
}

char *get_comment(char *args) {
        return (mpd_song && mpd_song->comment) ? mpd_song->comment : "\0";
}

char *get_albumartist(char *args) {
        return (mpd_song && mpd_song->albumartist) ? mpd_song->albumartist : "\0";
}

char *get_volume(char *args) {
        if (mpd_conn) {
                char *volume = NULL;
                asprintf(&volume, "%d", mpd_status_get_volume(mpd_conn));
                return m_freelater(volume);
        }

        return "0";
}

//char *get_repeat(char *args) { return (mpd_song->repeat) ? mpd_song->repeat : "\0"; }
//char *get_random(char *args) { return (mpd_song->random) ? mpd_song->random : "\0"; }
//char *get_playlist(char *args) { return (mpd_song->playlist) ? mpd_song->playlist : "\0"; }
//char *get_playlistlength(char *args) { return (mpd_song->playlistlength) ? mpd_song->playlistlength : "\0"; }
//char *get_xfade(char *args) { return (mpd_song->xfade) ? mpd_song->xfade : "\0"; }
//char *get_song(char *args) { return (mpd_song->song) ? mpd_song->song : "\0"; }

char *get_bitrate(char *args) {
        if (mpd_conn) {
                char *bitrate = NULL;
                asprintf(&bitrate, "%d", mpd_status_get_bitrate(mpd_conn));
                return m_freelater(bitrate);
        }

        return "0";
}

//char *get_audio(char *args) { return (mpd_song->audio) ? mpd_song->audio : "\0"; }

char *get_elapsed_time(char *args) {
        if (mpd_conn) {
                int elapsed = mpd_status_get_elapsed_song_time(mpd_conn);
                char *ret = NULL;
                asprintf(&ret, "%.2d:%.2d", elapsed / 60, elapsed % 60);
                return m_freelater(ret);
        }

        return "00:00";
}

char *get_total_time(char *args) {
        if (mpd_conn) {
                int total = mpd_status_get_total_song_time(mpd_conn);
                char *ret = NULL;
                asprintf(&ret, "%.2d:%.2d", total / 60, total % 60);
                return m_freelater(ret);
        }

        return "00:00";
} 

int get_volume_bar(char *args) {
        if (mpd_conn) {
                int volume = mpd_status_get_volume(mpd_conn);
                if (volume >= 0)
                        return volume;
        }

        return 0;
}

