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

#ifndef DAEMON_H
#define DAEMON_H

#include <stdbool.h>

struct donky_conn_node {
        int sock;
        
        bool authed;
        
        struct donky_conn_node *next;
        struct donky_conn_node *prev;
};

typedef struct donky_conn_node donky_conn;

void donky_loop(void);
void donky_conn_drop(donky_conn *cur);

#endif /* DAEMON_H */
