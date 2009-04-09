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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <linux/major.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* The following ifdefs were adapted from gkrellm */
#if !defined(MD_MAJOR)
#define MD_MAJOR 9
#endif

#if !defined(LVM_BLK_MAJOR)
#define LVM_BLK_MAJOR 58
#endif

#if !defined(NBD_MAJOR)
#define NBD_MAJOR 43
#endif

struct diskio_stat {
	struct diskio_stat *next;
	char *dev;
	unsigned int current;
	unsigned int current_read;
	unsigned int current_write;
	unsigned int last;
	unsigned int last_read;
	unsigned int last_write;
};

/* this is the root of all per disk stats,
 * also containing the totals. */
static struct diskio_stat stats = {
	.next = NULL,
	.current = 0,
	.current_read = 0,
	.current_write = 0,
	.last = UINT_MAX,
	.last_read = UINT_MAX,
	.last_write = UINT_MAX,
};

/* Module name */
char module_name[] = "diskio";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_diskio(char *args);
void clear_diskio_stats(void);
struct diskio_stat *prepare_diskio_stat(const char *s);
static void update_diskio_values(struct diskio_stat *ds,
		                 unsigned int reads,
                                 unsigned int writes);
void update_diskio(void);

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "diskio", "get_diskio", 1.0, VARIABLE_STR);
        module_var_cron_add(module_name, "diskio_cron", "update_diskio", 1.0, VARIABLE_CRON);
}

char *get_diskio(char *args)
{
        return m_freelater(bytes_to_bigger(stats.current * 1024));
}

/* These run on module unload */
void module_destroy(void)
{
        clear_diskio_stats();
}

void clear_diskio_stats(void)
{
	struct diskio_stat *cur;
	while (stats.next) {
		cur = stats.next;
		stats.next = stats.next->next;
		free(cur);
	}
}

struct diskio_stat *prepare_diskio_stat(const char *s)
{
	struct diskio_stat *cur = &stats;

	if (!s)
		return &stats;

	/* lookup existing */
	while (cur->next) {
		cur = cur->next;
		if (!strcmp(cur->dev, s))
			return cur;
	}

	/* no existing found, make a new one */
	cur->next = malloc(sizeof(struct diskio_stat));
	cur = cur->next;
	memset(cur, 0, sizeof(struct diskio_stat));
	cur->dev = d_strncpy(s, sizeof(s));
	cur->last = UINT_MAX;
	cur->last_read = UINT_MAX;
	cur->last_write = UINT_MAX;
	return cur;
}

static void update_diskio_values(struct diskio_stat *ds,
		unsigned int reads, unsigned int writes)
{
	if (reads < ds->last_read || writes < ds->last_write) {
		/* counter overflow or reset - rebase to sane values */
		ds->last = 0;
		ds->last_read = 0;
		ds->last_write = 0;
	}
	/* since the values in /proc/diskstats are absolute, we have to substract
	 * our last reading. The numbers stand for "sectors read", and we therefore
	 * have to divide by two to get KB */
	ds->current_read = (reads - ds->last_read) / 2;
	ds->current_write = (writes - ds->last_write) / 2;
	ds->current = ds->current_read + ds->current_write;

	ds->last_read = reads;
	ds->last_write = writes;
	ds->last = ds->last_read + ds->last_write;
}

void update_diskio(void)
{
	FILE *fp;

	struct diskio_stat *cur;
	char buf[512], devbuf[64];
	unsigned int major, minor;
	unsigned int reads, writes;
	unsigned int total_reads, total_writes;
	int col_count = 0;

	stats.current = 0;
	stats.current_read = 0;
	stats.current_write = 0;

	if (!(fp = fopen("/proc/diskstats", "r")))
		return;

	/* read reads and writes from all disks (minor = 0), including cd-roms
	 * and floppies, and sum them up */
	while (fgets(buf, 512, fp)) {
		col_count = sscanf(buf, "%u %u %s %*u %*u %u %*u %*u %*u %u", &major,
			&minor, devbuf, &reads, &writes);
		/* ignore subdevices (they have only 3 matching entries in their line)
		 * and virtual devices (LVM, network block devices, RAM disks, Loopback)
		 *
		 * XXX: ignore devices which are part of a SW RAID (MD_MAJOR) */
		if (col_count == 5 && major != LVM_BLK_MAJOR && major != NBD_MAJOR
				&& major != RAMDISK_MAJOR && major != LOOP_MAJOR) {
			total_reads += reads;
			total_writes += writes;
		} else {
			col_count = sscanf(buf, "%u %u %s %*u %u %*u %u",
				&major, &minor, devbuf, &reads, &writes);
			if (col_count != 5) {
				continue;
			}
		}
		cur = stats.next;
		while (cur && strcmp(devbuf, cur->dev))
			cur = cur->next;

		if (cur)
			update_diskio_values(cur, reads, writes);
	}
	update_diskio_values(&stats, total_reads, total_writes);
	fclose(fp);
}

