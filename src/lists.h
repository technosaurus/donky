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

struct list_item {
        void *data;
        struct list_item *next;
        struct list_item *prev;
};

struct list {
        struct list_item *first;
        struct list_item *last;
};

struct list *init_list(void);
void *add_node(struct list *fl, void *data);
void *get_node(struct list *fl,
               void *match_callback,
               void *match,
               void *fallback);
void *find_node(struct list *fl, void *match_callback, void *match);
void del_node(struct list *fl,
              void *match_callback,
              void *match,
              void *free_external);
void del_nodes(struct list *fl,
               void *match_callback,
               void *match,
               void *free_external);
void del_list(struct list *fl, void *free_external);
void act_on_list(struct list *fl, void *execute);
void act_on_list_raw(struct list *fl, void *execute);
void act_on_list_if(struct list *fl, void *execute,
                    void *match_callback, void *match);
void act_on_list_raw_if(struct list *fl, void *execute,
                        void *match_callback, void *match);
