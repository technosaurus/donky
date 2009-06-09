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
#include <iwlib.h>

#include "../cfg.h"
#include "../mem.h"
#include "../module.h"
#include "../util.h"

char module_name[] = "wifi_pwn_edition";

/* Globals */
static const char *interface;

static struct wifi_info {
        char essid[64];
        char mode[32];
        char bitrate[16];
        char ap[64];
        char link_qual[8];
        char link_qual_max[8];
        char link_qual_perc[4];
} wifistuff;

/**
 * @brief This is run on module initialization.
 */
void module_init(const struct module *mod)
{
        module_var_add(mod, "wifi_essid", "get_essid", 5.0, VARIABLE_STR);
        module_var_add(mod, "wifi_mode", "get_mode", 5.0, VARIABLE_STR);
        module_var_add(mod, "wifi_bitrate", "get_bitrate", 5.0, VARIABLE_STR);
        module_var_add(mod, "wifi_ap", "get_ap", 5.0, VARIABLE_STR);
        module_var_add(mod, "wifi_link_qual", "get_link_qual", 5.0, VARIABLE_STR);
        module_var_add(mod, "wifi_link_qual_max", "get_link_qual_max", 5.0, VARIABLE_STR);
        module_var_add(mod, "wifi_link_qual_perc", "get_link_qual_perc", 5.0, VARIABLE_STR);
        
        module_var_add(mod, "wifi_link_bar", "get_link_bar", 5.0, VARIABLE_BAR);

        /* Add cron job. */
        module_var_add(mod, "wifi_cron", "run_cron", 5.0, VARIABLE_CRON);

        interface = get_char_key("wifi", "interface", "wlan0");
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
}

/**
 * @brief This is run every cron timeout.
 * 
 *        The code within this method was copied almost character for character
 *        from src/linux.c of conky-1.6.1!  I found in the AUTHORS file of conky
 *        that the Linux wifi code was submitted by:
 *           * Toni Spets hifi <spets at users dot sourceforge dot net>
 */
void run_cron(void)
{
        wireless_info *info;
        struct iwreq wrq;
        int skfd;
        double percent;

        info = calloc(1, sizeof(wireless_info));
        skfd = iw_sockets_open();
        
        if (iw_get_basic_config(skfd, interface, &(info->b)) > -1) {
                if (iw_get_stats(skfd, interface, &(info->stats),
                                 &info->range, info->has_range) >= 0) {
                        info->has_stats = 1;
                }
                if (iw_get_range_info(skfd, interface, &(info->range)) >= 0) {
                        info->has_range = 1;
                }
                if (iw_get_ext(skfd, interface, SIOCGIWAP, &wrq) >= 0) {
                        info->has_ap_addr = 1;
                        memcpy(&(info->ap_addr), &(wrq.u.ap_addr), sizeof(sockaddr));
                }

                /* get bitrate */
                if (iw_get_ext(skfd, interface, SIOCGIWRATE, &wrq) >= 0) {
                        memcpy(&(info->bitrate), &(wrq.u.bitrate), sizeof(iwparam));
                        iw_print_bitrate(wifistuff.bitrate,
                                         sizeof(wifistuff.bitrate),
                                         info->bitrate.value);
                }

                /* get link quality */
                if (info->has_range && info->has_stats &&
                    ((info->stats.qual.level != 0) ||
                    (info->stats.qual.updated & IW_QUAL_DBM))) {
                        if (!(info->stats.qual.updated & IW_QUAL_QUAL_INVALID)) {
                                snprintf(wifistuff.link_qual,
                                         sizeof(wifistuff.link_qual),
                                         "%d",
                                         info->stats.qual.qual);
                                snprintf(wifistuff.link_qual_max,
                                         sizeof(wifistuff.link_qual_max),
                                         "%d",
                                         info->range.max_qual.qual);

                                percent = ((double) info->stats.qual.qual /
                                           (double) info->range.max_qual.qual) *
                                          100.0;
                                snprintf(wifistuff.link_qual_perc,
                                         sizeof(wifistuff.link_qual_perc),
                                         "%d",
                                         (int) percent);
                        }
                }

                /* get ap mac */
                if (info->has_ap_addr)
                        iw_sawap_ntop(&info->ap_addr, wifistuff.ap);

                /* get essid */
                if (info->b.has_essid) {
                        if (info->b.essid_on)
                                snprintf(wifistuff.essid,
                                         sizeof(wifistuff.essid),
                                         "%s", info->b.essid);
                        else
                                snprintf(wifistuff.essid,
                                         sizeof(wifistuff.essid),
                                         "off/any");
                }

                snprintf(wifistuff.mode, sizeof(wifistuff.mode),
                         "%s", iw_operation_mode[info->b.mode]);
        }
        
        iw_sockets_close(skfd);
        free(info);
}

char *get_essid(char *args) { return m_strdup(wifistuff.essid); }
char *get_mode(char *args) { return m_strdup(wifistuff.mode); }
char *get_bitrate(char *args) { return m_strdup(wifistuff.bitrate); }
char *get_ap(char *args) { return m_strdup(wifistuff.ap); }
char *get_link_qual(char *args) { return m_strdup(wifistuff.link_qual); }
char *get_link_qual_max(char *args) { return m_strdup(wifistuff.link_qual_max); }
char *get_link_qual_perc(char *args) { return m_strdup(wifistuff.link_qual_perc); }
unsigned int get_link_bar(char *args) { return strtol(wifistuff.link_qual_perc, NULL, 0); }
