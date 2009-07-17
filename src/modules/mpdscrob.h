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

#ifndef MPDSCROB_H
#define MPDSCROB_H

extern int scrob_enabled;       /* bool */

void scrob_init(void);
void scrob_urself(const char *artist, const char *title, const char *album,
                  const char *track, const int etime, const int ttime);

#endif /* MPDSCROB_H */
