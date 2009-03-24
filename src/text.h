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

#ifndef TEXT_H
#define TEXT_H

enum text_section_type {
        TEXT_STATIC,    /* This is static text.  It does not need to be evaluated. */
        TEXT_VARIABLE,  /* Needs further evaluation before drawing! */
        TEXT_FONT,      /* This is a font, args will be either a font name or NULL. */
        TEXT_COLOR      /* Color, args will be a color name/hex val or NULL. */
};

struct text_section {
        char *value;                    /* Value of this text section. */
        char *args;                     /* Arguments given to a variable. ${...} */
        
        unsigned int line;              /* Line number, starts at 0. */
        unsigned int pixel_width;       /* Pixel width of this section. */
        unsigned int old_pixel_width;   /* Old width of this section. */
        int16_t xpos;                   /* Current x position. */
        int16_t ypos;                   /* Current y position. */
        int16_t old_xpos;               /* Old x position. */
        int16_t old_ypos;               /* Old y position. */

        double timeout;                 /* How often to check for updates. */
        double last_update;             /* Time of the last update. */

        enum text_section_type type;    /* See the definition of this enum in text.h */

        struct module_var *mod_var;     /* Pointer to corresponding module var. */

        struct text_section *next;      /* Next node in this linked list. */
        struct text_section *prev;      /* Previous node in linked list. */
};

struct text_section *ts_start;
struct text_section *ts_end;

void parse_text(void);
void clear_text(void);

struct text_section *text_section_var_find(char *value);
void text_section_var_modvar(char *value,
                             struct module_var *mvar,
                             double timeout);

#endif /* TEXT_H */
