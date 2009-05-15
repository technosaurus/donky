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

#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "daemon.h"
#include "default_settings.h"
#include "mem.h"
#include "module.h"
#include "protocol.h"
#include "request.h"
#include "util.h"

/* Globals. */
struct request_list *rl_start = NULL;
struct request_list *rl_end = NULL;
static pthread_t request_thread_id;

/* Function prototypes. */
static void request_handler_sleep_setup(struct timespec *tspec);
static void *request_handler_exec(void *arg);
static bool thread_launched = false;

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

        thread_launched = true;

        return 1;
}

/**
 * @brief Kill the request handler thread.
 */
void request_handler_stop(void)
{
        if (thread_launched) {
                pthread_cancel(request_thread_id);
                pthread_join(request_thread_id, NULL);
        }
}

/**
 * @brief Request handler execution thread.
 *
 * @param arg Arguments
 */
static void *request_handler_exec(void *arg)
{
        struct timespec tspec;
        request_handler_sleep_setup(&tspec);
        struct request_list *cur;
        struct request_list *next;
        double timeout;
        double last_update;
        char *(*func_str)(char *);
        unsigned int (*func_int)(char *);
        char *ret_str;
        int ret_int;
        unsigned int sum;
        int n;

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
                            (cur->remove != true) &&
                            (cur->first != true)) {
                                cur = next;
                                continue;
                        }

                        /* Figure out what type of variable this is. */
                        switch (cur->var->type) {
                        case VARIABLE_STR:
                                if ((func_str = cur->var->sym)) {
                                        ret_str = func_str(cur->args);
                                        sum = get_str_sum(ret_str);

                                        if (sum != cur->var->sum || cur->remove) {
                                                printf("Sending str\n");
                                                n = sendcrlf(cur->conn->sock,
                                                             "%s:%d:%s",
                                                             cur->id,
                                                             cur->var->type,
                                                             ret_str);
                                                cur->var->sum = sum;

                                                if (n <= 0) {
                                                        printf("Removing...\n");
                                                        cur->remove = true;
                                                }
                                        }
                                }
                                break;
                        case VARIABLE_BAR:
                        case VARIABLE_GRAPH:
                                if ((func_int = cur->var->sym)) {
                                        ret_int = func_int(cur->args);
                                        sum = ret_int;

                                        if (sum != cur->var->sum || cur->remove) {
                                                printf("Sending bar or graph\n");
                                                n = sendcrlf(cur->conn->sock,
                                                             "%s:%d:%d",
                                                             cur->id,
                                                             cur->var->type,
                                                             ret_int);
                                                cur->var->sum = sum;

                                                if (n <= 0) {
                                                        printf("Removing...\n");
                                                        cur->remove = true;
                                                }
                                        }
                                }
                                break;
                        default:
                                break;
                        }

                        /* Set the last time it was updated. */
                        cur->var->last_update = get_time();

                        /* Remove this request. */
                        if (cur->remove)
                                request_list_remove(cur);

                        /* This isn't the first handle anymore. */
                        if (cur->first)
                                cur->first = false;

                        /* Clear memory list. */
                        mem_list_clear();
                        
                        /* Next node. */
                        cur = next;
                }

                /* Sleep! */
                if (nanosleep(&tspec, NULL) == -1) {
                        printf("Breaking from request handler...\n");
                        break;
                }
        }

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
        min_seconds = floor(min_sleep);
        min_nanoseconds = (min_sleep - min_seconds) * pow(10, 9);
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
int request_list_add(const donky_conn *conn, const char *buf, bool remove)
{
        struct request_list *n;
        char *id;
        char *var;
        char *args;
        struct module_var *mv;

        id = d_strcpy(buf);

        /* Get the variable name... */
        var = strchr(id, ':');
        if (!var)
                return 0;

        *var = '\0';
        var++;

        /* Get the arguments... */
        args = strchr(var, ' ');
        if (args) {
                *args = '\0';
                args++;
        }

        printf("Request list add: id[%s] var[%s] args[%s]\n", id, var, args);

        /* Find the module_var node for this variable. */
        if ((mv = module_var_find_by_name(var)) == NULL) {
                printf("Couldn't find module var!\n");
                return 0;
        }

        /* Create request_list node... */
        n = malloc(sizeof(struct request_list));

        n->id = id;
        n->conn = conn;
        n->var = mv;
        n->args = args;
        n->remove = remove;
        n->first = true;

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
        if (!mv->sym)
                mv->sym = module_get_sym(mv->parent->handle, mv->method);

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

        printf("Removing from request list...\n");

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

        freeif(cur->id);
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

                freeif(cur->id);
                free(cur);
                
                cur = next;
        }

        rl_start = NULL;
        rl_end = NULL;
        thread_launched = false;
}
