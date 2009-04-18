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
void *add_node(struct first_last *fl, void *data);
void *get_node(struct first_last *fl,
               void *match_callback,
               void *match,
               void *fallback);
void *find_node(struct first_last *fl, void *match_callback, void *match);
void del_node(struct first_last *fl,
              void *match_callback,
              void *match,
              void *free_external);
void del_list(struct first_last *fl, void *free_external);
void act_on_list(struct first_last *fl, void *execute);
void act_on_list_raw(struct first_last *fl, void *execute);
void act_on_list_if(struct first_last *fl, void *execute,
                    void *match_callback, void *match);
void act_on_list_raw_if(struct first_last *fl, void *execute,
                        void *match_callback, void *match);
