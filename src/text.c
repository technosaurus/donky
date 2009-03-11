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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"

#define MAX_TEXT_SIZE 1024
#define IS_ALPHA(c) ( \
        ((c) >= 'a' && (c) <= 'z') || \
        ((c) >= 'A' && (c) <= 'Z') || \
        ((c) >= '0' && (c) <= '9') || \
        ((c) == '_') \
)

const char *config_text = "($time) | $$5 $$ [$command] arg1\\ arg2 Testing and |${shit 0 0}| $mpd_smart\nHello world!\n$test:$test2 \\\nTest.\nfarting and washing\\\nturds\n";

enum text_section_type {
        TEXT_STATIC,
        TEXT_VARIABLE
};

struct text_section {
        char *value;
        int line;

        enum text_section_type type;

        struct text_section *next;
} *ts_start = NULL, *ts_end = NULL;

void text_section_add(char *, int, int, enum text_section_type);
void parse_text(void);

/**
 * @brief Parse the output text into lines and sections.
 */
void parse_text(void)
{
        char *dup;
        char *s;
        char *ts;
        int line = 0;

        dup = strndup(config_text, MAX_TEXT_SIZE);
        s = ts = dup;

        for (; *s; s++) {
                if (*s == '\n') {
                        /* New line cancellation. */
                        if (*(s - 1) == '\\') {
                                *(s - 1) = '\0';
                                text_section_add(ts, s - ts, line, TEXT_STATIC);
                                ts = s + 1;
                                continue; 
                        }
                        
                        /* New lines mark the end of a section, so add
                         * all data preceding it as a section. */
                        if ((s - ts) > 0)
                                text_section_add(ts, s - ts, line, TEXT_STATIC);

                        /* Update the pointer to the beginning of the
                         * next text section. */
                        ts = s + 1;

                        line++;
                } else if (*s == '$') {
                        /* $$ will be just $ */
                        if (*(s + 1) == '$') {
                                text_section_add(ts, s - ts, line, TEXT_STATIC);
                                s++;
                                ts = s;
                                continue;
                        }
                        
                        /* If we have a section of text before this
                         * variable, lets add it to the section list. */
                        if ((s - ts) > 0)
                                text_section_add(ts, s - ts, line, TEXT_STATIC);

                        /* Update the pointer to the beginning of a
                         * text section. */
                        s++;
                        ts = s;

                        /* ${...} variables. */
                        if (*s == '{') {
                                s++;
                                ts = s;

                                for (; *s &&
                                     (IS_ALPHA(*s) || *s == ' ') &&
                                     *s != '}'; s++) { }

                                if ((s - ts) > 0) {
                                        text_section_add(ts, s - ts,
                                                         line,
                                                         TEXT_VARIABLE);
                                        ts = s + 1;
                                }
                        /* Regular $... variables. */
                        } else {
                                for (; *s && IS_ALPHA(*s); s++) { }

                                if ((s - ts) > 0) {
                                        text_section_add(ts, s - ts,
                                                         line,
                                                         TEXT_VARIABLE);
                                        ts = s;
                                }

                                s--;
                        }
                }
        }

        free(dup);

        struct text_section *cur = ts_start;

        while (cur != NULL) {
                printf("LINE %d STR = [%s]\n", cur->line, cur->value);
                cur = cur->next;
        }
}

/**
 * @brief Add a text section to the text_section linked list.
 *
 * @param str Pointer to the string of text
 * @param len Length of the string
 * @param line Current line number
 * @param type Section type (TEXT_VARIABLE, TEXT_STATIC)
 */
void text_section_add(char *value, int len, int line, enum text_section_type type)
{
        struct text_section *n = malloc(sizeof(struct text_section));
        
        n->value = malloc(sizeof(char) * len);
        strncpy(n->value, value, len);
        n->line = line;
        n->type = type;
        n->next = NULL;

        if (ts_start == NULL) {
                ts_start = n;
                ts_end = n;
        } else {
                ts_end->next = n;
                ts_end = n;
        }
}
