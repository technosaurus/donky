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
#include <alsa/asoundlib.h>

#include "../std/string.h"

#include "../mem.h"
#include "../module.h"

/* Module name */
char module_name[] = "volume";

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

/* These run on module startup */
void module_init(struct module *mod)
{
        module_var_add(mod, "volume", "get_volume", 5.0, VARIABLE_STR);
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
        
        int volume_alsaSet = -1;
       
        ret_volume = NULL;

	if (prep_alsa_get_level(&level, args) < 0)
                return "n/a";

        max = volume_alsaMax;
	min = volume_alsaMin;

	ret = ((volume_alsaSet / 100.0) * (max - min) + min) + 0.5;
	if (volume_alsaSet > 0 && ret == level)
		ret = volume_alsaSet;
	else
		ret = (int)(100 * (((float)(level - min)) / (max - min)) + 0.5);

        close_alsa_mixer();

        ret_volume = m_malloc(5 * sizeof(char));
        snprintf(ret_volume, (4 * sizeof(char)), "%d", ret);

	return ret_volume;
}

int prep_alsa_get_level(long *level, char *args)
{
        const char *volume_mixerDevice = "default";

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

