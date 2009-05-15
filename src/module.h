/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MODULE_H
#define MODULE_H

enum variable_type {
        VARIABLE_STR,   /* Function should return char * */
        VARIABLE_BAR,   /* Function should return int between 0 and 100 */
        VARIABLE_GRAPH, /* Function should return int between 0 and 100 */
        VARIABLE_CRON   /* This is simply a method to be run on schedule. */
};

struct module {
        char name[64];  /* Unique identifier, value really doesn't matter. */
        char *path;     /* Path to the module file. */
        void *handle;   /* Handle to the code in memory. */
        void *destroy;  /* Pointer to the module_destroy function. */
        int clients;    /* How many clients are using this module? */

        struct module *next;
        struct module *prev;
};

struct module_var {
        char name[64];           /* Name of the variable. */
        char method[64];         /* Method name to call. */
        void *sym;               /* Link to method symbol. */
        enum variable_type type; /* Type of method.  See the enum above. */

        double timeout;          /* Used for cron jobs */
        double last_update;      /* Ditto */

        unsigned int sum;        /* Sum of chars or what have you. */

        struct module *parent;   /* Parent of this module. */

        struct module_var *next;
        struct module_var *prev;
};

/* Function prototypes. */
int module_var_add(const struct module *parent,
                   char *name,
                   const char *method,
                   double timeout,
                   enum variable_type type);
void module_load_all(void);
void clear_module(void);
void module_var_cron_exec(void);
void *module_get_sym(void *handle, char *name);
struct module_var *module_var_find_by_name(const char *name);
int module_load(char *path);
void module_unload(struct module *cur);
void module_var_cron_init(struct module *parent);

#endif /* MODULE_H */

