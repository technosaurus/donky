/**
 * The CC0 1.0 Universal is applied to this work.
 *
 * To the extent possible under law, Matt Hayes and Jake LeMaster have
 * waived all copyright and related or neighboring rights to donky.
 * This work is published from the United States.
 *
 * Please see the copy of the CC0 included with this program for complete
 * information including limitations and disclaimers. If no such copy
 * exists, see <http://creativecommons.org/publicdomain/zero/1.0/legalcode>.
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

