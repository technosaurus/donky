/**
 * The CC0 1.0 Universal is applied to this work.
 *
 * To the extent possible under law, Matt Hayes and Jake LeMaster have
 * waived all copyright and related or neighboring rights to donky.
 * This work is published from the United States.
 *
 * Please see the copy of the CC0 included with this program for complete
 * information including limitations and disclaimers. If no such copy
 * exists, see <http://creativecommons.org/publicdomain/zero/1.0/legalcode>.
 */

#ifndef MODULE_H
#define MODULE_H

#define VARIABLE_STR 1   /* Function should return char * */
#define VARIABLE_BAR 2   /* Function should return int between 0 and 100 */
#define VARIABLE_GRAPH 4 /* Function should return int between 0 and 100 */
#define VARIABLE_CRON 8  /* void function run on schedule */
#define ARGSTR 16        /* Function takes a char * argument */
#define ARGINT 32        /* Function takes an int argument */
#define ARGDOUBLE 64     /* Function takes a double argument */

struct module {
        char name[64];  /* Unique identifier, value really doesn't matter. */
        char *path;     /* Path to the module file. */
        void *handle;   /* Handle to the code in memory. */
        void *destroy;  /* Pointer to the module_destroy function. */
        int clients;    /* How many clients are using this module? */

        struct module *next;
        struct module *prev;
};

union module_funcs {
        void (*f_void)(void);
        
        char *(*f_str)(void);
        char *(*f_str_str)(char *);
        char *(*f_str_int)(int);
        char *(*f_str_double)(double);
        
        unsigned int (*f_int)(void);
        unsigned int (*f_int_str)(char *);
        unsigned int (*f_int_int)(int);
        unsigned int (*f_int_double)(double);
};

struct module_var {
        char name[64];           /* Name of the variable. */
        char method[64];         /* Method name to call. */
        union module_funcs syms;
        unsigned char type;      /* Type of method.  See the enum above. */
        
        int loaded;              /* Loaded (bool) */
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
                   unsigned char type);
void module_load_all(void);
void clear_module(void);
void module_var_cron_exec(void);
void *module_get_sym(void *handle, char *name);
struct module_var *module_var_find_by_name(const char *name);
void module_var_loadsym(struct module_var *mv);
int module_load(char *path);
void module_unload(struct module *cur);
void module_var_cron_init(struct module *parent);

#endif /* MODULE_H */

