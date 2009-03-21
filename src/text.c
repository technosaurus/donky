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
#include "config.h"
#include "util.h"

#define MAX_TEXT_SIZE 10240

#define IS_ALPHA(c) ( \
        ((c) >= 'a' && (c) <= 'z') || \
        ((c) >= 'A' && (c) <= 'Z') || \
        ((c) >= '0' && (c) <= '9') || \
        ((c) == '_') \
)

/* Function prototypes. */
void clear_text(void);
void text_section_split(char *text);
void text_section_add(char *value, int len, int line, enum text_section_type type);
struct text_section *text_section_var_find(char *value);
void parse_text(void);

/* Globals. */
ts_start = NULL;
ts_end = NULL;

/**
 * @brief Start up the output text parsing.
 */
void parse_text(void)
{
        text_section_split(config_text);

        /* We don't need this anymore! */
        free(config_text);

        struct text_section *cur = ts_start;

        while (cur != NULL) {
                printf("LINE %d STR = [%s], ARGS = [%s]\n", cur->line, cur->value, cur->args);
                cur = cur->next;
        }
}

/**
 * @brief Split text into sections based on variables and other special symbols.
 *
 * @param text Character array pointer of text to parse.
 */
void text_section_split(char *text)
{
        char *dup;
        char *s;
        char *ts;
        unsigned int line = 0;

        dup = strndup(text, MAX_TEXT_SIZE);
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
                                if ((s - ts) > 0)
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

                                for (; *s && *s != '}'; s++) { }

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

        /* Add remaining text... */
        if ((s - ts) > 0)
                text_section_add(ts, s - ts,
                                 line,
                                 TEXT_STATIC);

        free(dup);
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
        char *copy_val = strndup(value, len);
        char *alias_contents = get_char_key("alias", copy_val);
        char *args = NULL;
        
        if (type == TEXT_VARIABLE && alias_contents) {
                text_section_split(alias_contents);
                free(copy_val);
        } else {
                struct text_section *n = malloc(sizeof(struct text_section));

                /* We need to check if this string is all spaces so that the
                 * arg splitting code doesn't null it. */
                if (type == TEXT_VARIABLE && !is_all_spaces(copy_val)) {
                        /* Split value from arguments. */
                        args = strchr(copy_val, ' ');

                        /* Trim the arguments list. */
                        if (args) {
                                *args = '\0';
                                args++;
                                args = trim(args);
                        }
                }

                /* Fill in the text_section structure. */
                n->value = copy_val;
                n->args = args;
                n->line = line;
                n->next = NULL;

                /* Set the type.  We do a little interception here to find
                 * some built-in variables. */
                if (!strcmp(n->value, "font"))
                        n->type = TEXT_FONT;
                else if (!strcmp(n->value, "color"))
                        n->type = TEXT_COLOR;
                else
                        n->type = type;

                /* Add to linked list. */
                if (ts_start == NULL) {
                        ts_start = n;
                        ts_end = n;
                } else {
                        ts_end->next = n;
                        ts_end = n;
                }
        }
}

/**
 * @brief Find a text section of TEXT_VARIABLE type by value.
 *
 * @param value Value string
 *
 * @return Text section node.
 */
struct text_section *text_section_var_find(char *value)
{
        struct text_section *cur = ts_start;

        while (cur != NULL) {
                if (cur->type == TEXT_VARIABLE && !strcmp(cur->value, value))
                    return cur;
                
                cur = cur->next;
        }
        
        return NULL;
}

/**
 * @brief Clear out the text sections linked list and free any allocated memory.
 */
void clear_text(void)
{
        struct text_section *cur = ts_start, *to_free;

        while (cur != NULL) {
                to_free = cur;
                cur = cur->next;

                if (to_free->value)
                        free(to_free->value);
                
                free(to_free);
        }

        ts_start = NULL;
        ts_end = NULL;
}
