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

struct list {
        void *data;
        struct list *next;
        struct list *prev;
};

struct first_last {
        struct list *first;
        struct list *last;
};

struct first_last *init_list(void);
void *add_node(void *data, struct first_last *fl);
void *get_node(void *match_callback,
                      void *fallback,
                      void *match,
                      struct first_last *fl);
void del_node(void *match_callback,
              void *free_external,
              void *match,
              struct first_last *fl);
void del_list(void *free_external, struct first_last *fl);
void act_on_list(void *execute, struct first_last *fl);

