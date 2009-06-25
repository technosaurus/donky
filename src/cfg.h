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

#ifndef CONFIG_H
#define CONFIG_H

void parse_cfg(void);
void clear_cfg(void);

/**
 * These functions are to be used by donky modules to look up user settings
 * from the donky configuration file. Use the function that returns the value
 * of the setting in the type appropriate for what you need. get_char_key()
 * returns the value as type char *, get_int_key() returns the value as type
 * int, etc.
 *
 * Parameters:
 *      mod = the name of the module
 *      key = the name of the setting whose value you're interested in
 *      otherwise = if the setting doesn't exist, this is returned
 *
 * So, if the mpd module is interested in what port it should try to talk to
 * mpd with, the module will do something like...
 *
 *      int port;
 *      port = get_int_key("mpd", "port", -1);
 *      if (port == -1)
 *              port = DEFAULT_MPD_PORT;
 *
 * This will see if in the user's donkyrc, there was a section like this...
 *
 *      [mpd]
 *      port = 6600
 *
 * If there wasn't, we told get_int_key to return -1 (the 'otherwise'
 * parameter). We'd then know the user didn't define a port and would use
 * the default.
 */
const char *get_char_key(const char *mod,
                         const char *key,
                         const char *otherwise);
int get_int_key(const char *mod, const char *key, int otherwise);
double get_double_key(const char *mod, const char *key, double otherwise);
int get_bool_key(const char *mod, const char *key, int otherwise);

#endif /* CONFIG_H */

