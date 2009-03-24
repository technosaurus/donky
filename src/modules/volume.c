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
#include <alsa/asoundlib.h>

#include "develop.h"
#include "../mem.h"

/* Module name */
char module_name[] = "volume";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_volume(char *args);
int prep_alsa_get_level(long *level, char *args);
void close_alsa_mixer(void);
int prep_alsa_mixer(const char *card, char *args);

/* Globals */
char *ret_volume = NULL;
snd_mixer_t *volume_alsaMixerHandle;
snd_mixer_elem_t *volume_alsaElem;
long volume_alsaMin;
long volume_alsaMax;
int volume_alsaSet = -1;
const char *volume_mixerDevice = "default";

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "volume", "get_volume", 5.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_volume(char *args)
{
	int ret;
	long level;
	long max;
	long min;
       
        ret_volume = NULL;

	if (prep_alsa_get_level(&level, args) < 0) {
		ret_volume = m_strdup("n/a");
                return ret_volume;
        }

        max = volume_alsaMax;
	min = volume_alsaMin;

        printf("VOLUME first\n");

	ret = ((volume_alsaSet / 100.0) * (max - min) + min) + 0.5;
	if (volume_alsaSet > 0 && ret == level) {
		ret = volume_alsaSet;
	} else
		ret = (int)(100 * (((float)(level - min)) / (max - min)) + 0.5);

        printf("VOLUME RET = %d\n", ret);

        ret_volume = m_malloc(5 * sizeof(char));
        snprintf(ret_volume, (4 * sizeof(char)), "%d", ret);

	return ret_volume;
}

int prep_alsa_get_level(long *level, char *args)
{
	if (!volume_alsaMixerHandle && prep_alsa_mixer(volume_mixerDevice, args) < 0)
		return -1;

	if ((snd_mixer_handle_events(volume_alsaMixerHandle)) < 0) {
	        close_alsa_mixer();
	        return -1;
	}
	if ((snd_mixer_selem_get_playback_volume(volume_alsaElem,
						 SND_MIXER_SCHN_FRONT_LEFT,
						 level)) < 0) {
	        close_alsa_mixer();
	        return -1;
	}
	return 0;
}

void close_alsa_mixer(void)
{
	snd_mixer_close(volume_alsaMixerHandle);
	volume_alsaMixerHandle = NULL;
}

int prep_alsa_mixer(const char *card, char *args)
{
	int err;
	snd_mixer_elem_t *elem;
	const char *controlName = "PCM";

	err = snd_mixer_open(&volume_alsaMixerHandle, 0);
	snd_config_update_free_global();
	if (err < 0)
		return -1;

	if ((snd_mixer_attach(volume_alsaMixerHandle, card)) < 0) {
		close_alsa_mixer();
		return -1;
	}

	if ((snd_mixer_selem_register(volume_alsaMixerHandle,
                                            NULL, NULL)) < 0) {
		close_alsa_mixer();
		return -1;
	}

	if ((snd_mixer_load(volume_alsaMixerHandle)) < 0) {
		close_alsa_mixer();
		return -1;
	}

	elem = snd_mixer_first_elem(volume_alsaMixerHandle);

	if (args) {
		controlName = args;
	}

	while (elem) {
		if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE) {
			if (strcasecmp(controlName,
				       snd_mixer_selem_get_name(elem)) == 0) {
				break;
			}
		}
		elem = snd_mixer_elem_next(elem);
	}

	if (elem) {
		volume_alsaElem = elem;
		snd_mixer_selem_get_playback_volume_range(volume_alsaElem,
							  &volume_alsaMin,
							  &volume_alsaMax);
		return 0;
	}

	close_alsa_mixer();
	return -1;
}

