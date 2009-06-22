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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../config.h"
#include "cfg.h"
#include "daemon.h"
#include "default_settings.h"
#include "mem.h"
#include "module.h"
#include "net.h"
#include "protocol.h"
#include "request.h"
#include "util.h"

/* Globals. */
struct request_list *rl_start = NULL;
struct request_list *rl_end = NULL;
static pthread_t request_thread_id;
static int thread_is_launched = 0; /* bool */

/* Function prototypes. */
static void request_handler_sleep_setup(struct timespec *tspec);
static void *request_handler_exec(void *arg);
static char *request_handler_strfunc(struct request_list *cur);
static unsigned int request_handler_intfunc(struct request_list *cur);

/**
 * @brief Start the request handler execution thread.
 */
int request_handler_start(void)
{
        int s;
        pthread_attr_t request_thread_attr;

        s = pthread_attr_init(&request_thread_attr);
        if (s != 0)
                return 0;

        s = pthread_create(&request_thread_id, &request_thread_attr,
                           &request_handler_exec, NULL);

        if (s != 0)
                return 0;

        /* We don't need this anymore! */
        pthread_attr_destroy(&request_thread_attr);

        thread_is_launched = 1;

        return 1;
}

/**
 * @brief Kill the request handler thread.
 */
void request_handler_stop(void)
{
        if (thread_is_launched) {
                pthread_cancel(request_thread_id);
                pthread_join(request_thread_id, NULL);
        }
}

/**
 * @brief Call module methods according to the argument type.
 *
 * @param cur Request list node
 *
 * @return String
 */
static char *request_handler_strfunc(struct request_list *cur)
{
        char *ret;
        
        if (cur->var->type & ARGSTR)
                ret = cur->var->syms.f_str_str(cur->args);
        else if (cur->var->type & ARGINT)
                ret = cur->var->syms.f_str_int((cur->args) ?
                                               atoi(cur->args) : -1);
        else if (cur->var->type & ARGDOUBLE)
                ret = cur->var->syms.f_str_double((cur->args) ?
                                                  strtod(cur->args, NULL) : -1.0);
        else
                ret = cur->var->syms.f_str();

        return ret;
}

/**
 * @brief Call module methods according to the argument type.
 *
 * @param cur Request list node
 *
 * @return Integer
 */
static unsigned int request_handler_intfunc(struct request_list *cur)
{
        unsigned int ret;
        
        if (cur->var->type & ARGSTR)
                ret = cur->var->syms.f_int_str(cur->args);
        else if (cur->var->type & ARGINT)
                ret = cur->var->syms.f_int_int((cur->args) ?
                                               atoi(cur->args) : -1);
        else if (cur->var->type & ARGDOUBLE)
                ret = cur->var->syms.f_int_double((cur->args) ?
                                                  strtod(cur->args, NULL) : -1.0);
        else
                ret = cur->var->syms.f_int();

        return ret;
}

/**
 * @brief Request handler execution thread.
 *
 * @param arg Arguments
 */
static void *request_handler_exec(void *arg)
{
        struct timespec tspec;
        struct request_list *cur;
        struct request_list *next;
        double timeout;
        double last_update;
        char *ret_str;
        unsigned int ret_int;
        unsigned int sum;
        int n;

        request_handler_sleep_setup(&tspec);

        /* Infinite Spewns Nerdiness Loop (tm) */
        while (1) {
                module_var_cron_exec();
                
                cur = rl_start;

                /* Handle all requests. */
                while (cur) {
                        next = cur->next;

                        timeout = cur->var->timeout;
                        last_update = cur->var->last_update;

                        /* Keep going cuz this one hasn't timed out. */
                        if ((get_time() - last_update < timeout) &&
                            (timeout != 0 && last_update != 0) &&
                            (cur->remove != 1) &&
                            (cur->is_first != 1)) {
                                cur = next;
                                continue;
                        }

                        /* Check that we have a symbol for the module var
                         * method. */
                        if (!module_var_checksym(cur->var))
                                goto UPDATESTAT;

                        /* VARIABLE_STR */
                        if (cur->var->type & VARIABLE_STR) {
                                ret_str = request_handler_strfunc(cur);
                                sum = get_str_sum(ret_str);

                                if (sum != cur->var->sum || cur->remove) {
                                        n = sendcrlf(cur->conn->sock,
                                                     "%u:%d:%s",
                                                     cur->id,
                                                     cur->var->type,
                                                     ret_str);
                                        cur->var->sum = sum;

                                        if (n <= 0) {
#ifdef ENABLE_DEBUGGING
                                                printf("Removing...\n");
#endif
                                                cur->remove = 1;
                                        }
                                }
                        /* VARIABLE_BAR || VARIABLE_GRAPH */
                        } else if (cur->var->type & VARIABLE_BAR ||
                                   cur->var->type & VARIABLE_GRAPH) {
                                ret_int = request_handler_intfunc(cur);
                                sum = ret_int;

                                if (sum != cur->var->sum || cur->remove) {
                                        n = sendcrlf(cur->conn->sock,
                                                     "%u:%d:%d",
                                                     cur->id,
                                                     cur->var->type,
                                                     ret_int);
                                        cur->var->sum = sum;

                                        if (n <= 0) {
#ifdef ENABLE_DEBUGGING
                                                printf("Removing...\n");
#endif
                                                cur->remove = 1;
                                        }
                                }
                        }

UPDATESTAT:
                        /* Set the last time it was updated. */
                        cur->var->last_update = get_time();

                        /* Clear memory list. */
                        mem_list_clear();

                        /* This isn't the first handle anymore. */
                        if (cur->is_first)
                                cur->is_first = 0;

                        /* Remove this request. */
                        if (cur->remove)
                                request_list_remove(cur);
                        
                        /* Next node. */
                        cur = next;
                }

                /* Sleep! */
                if (nanosleep(&tspec, NULL) == -1) {
#ifdef ENABLE_DEBUGGING
                        printf("Breaking from request handler...\n");
#endif
                        break;
                }
        }

#ifdef ENABLE_DEBUGGING
        printf("Done with thread!\n");
#endif
        return NULL;
}

/**
 * @brief Setup the nanosleep timespec structure.
 *
 * @param tspec
 */
static void request_handler_sleep_setup(struct timespec *tspec)
{
        double min_sleep; 
        int min_seconds;
        long min_nanoseconds;

        /* Setup minimum sleep time. */
        min_sleep = get_double_key("daemon", "global_sleep", DEFAULT_GLOBAL_SLEEP);
        min_seconds = (int) min_sleep;
        min_nanoseconds = (min_sleep - min_seconds) * pown(10, 9);
        tspec->tv_sec = min_seconds;
        tspec->tv_nsec = min_nanoseconds;
}

/**
 * @brief Add node to the request list.
 *
 * @param conn
 * @param buf
 * @param remove
 */
int request_list_add(const donky_conn *conn, const char *buf, int remove)
{
        struct request_list *n;
        unsigned int id;
        char *str;
        char *var;
        char *args;
        struct module_var *mv;

        str = dstrdup(buf);

        /* Get the variable name... */
        var = strchr(str, ':');
        if (!var) {
                free(str);
                return 0;
        }

        *var = '\0';
        var++;

        /* Get the arguments... */
        args = strchr(var, ' ');
        if (args) {
                *args = '\0';
                args++;
        }

        id = atoi(str);
#ifdef ENABLE_DEBUGGING
        printf("Request list add: id[%u] var[%s] args[%s]\n", id, var, args);
#endif

        /* Find the module_var node for this variable. */
        if ((mv = module_var_find_by_name(var)) == NULL) {
#ifdef ENABLE_DEBUGGING
                printf("Couldn't find module var!\n");
#endif

                /* Send an error response. */
                sendcrlf(conn->sock, "%u:404:", id);
                
                free(str);
                return 0;
        }

        /* Create request_list node... */
        n = malloc(sizeof(struct request_list));

        n->id = id;
        n->conn = conn;
        n->var = mv;
        n->args = args;
        n->remove = remove;
        n->is_first = 1;
        n->tofree = str;

        n->prev = NULL;
        n->next = NULL;

        /* Add to linked list. */
        if (rl_end == NULL) {
                rl_start = n;
                rl_end = n;
        } else {
                rl_end->next = n;
                n->prev = rl_end;
                rl_end = n;
        }

        /* If the module currently has 0 clients, that means it isn't loaded,
         * so lets load it. */
        if (mv->parent->clients == 0) {
                module_load(mv->parent->path);
                module_var_cron_init(mv->parent);
        }

        /* If the symbol is NULL, get a pointer to it. */
        if (!module_var_checksym(mv))
                module_var_loadsym(mv);

        /* Let the module var parent know we are using it. */
        mv->parent->clients++;

        return 1;
}

/**
 * @brief Remove request list node.
 *
 * @param cur
 */
void request_list_remove(struct request_list *cur)
{
        if (!cur)
                return;
        
        cur->var->parent->clients--;

#ifdef ENABLE_DEBUGGING
        printf("Removing from request list...\n");
#endif

        /* If the clients just hit 0, unload this module. */
        if (cur->var->parent->clients == 0)
                module_unload(cur->var->parent);

        /* Remove node from linked list. */
        if (cur->prev)
                cur->prev->next = cur->next;
        if (cur->next)
                cur->next->prev = cur->prev;
        if (cur == rl_start)
                rl_start = cur->next;
        if (cur == rl_end)
                rl_end = cur->prev;

        free(cur->tofree);
        free(cur);
}

/**
 * @brief Find request list node by donky_conn
 *
 * @param conn Connection
 *
 * @return Request list node
 */
struct request_list *request_list_find_by_conn(donky_conn *conn)
{
        struct request_list *cur = rl_start;

        while (cur) {
                if (cur->conn == conn)
                        return cur;
                
                cur = cur->next;
        }

        return NULL;
}

/**
 * @brief Clear the request list.
 */
void request_list_clear(void)
{
        struct request_list *cur = rl_start;
        struct request_list *next;

        request_handler_stop();

        while (cur) {
                next = cur->next;

                free(cur->tofree);
                free(cur);
                
                cur = next;
        }

        rl_start = NULL;
        rl_end = NULL;
        thread_is_launched = 0;
}
