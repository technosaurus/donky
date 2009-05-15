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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "daemon.h"
#include "module.h"
#include "protocol.h"
#include "request.h"
#include "util.h"

/* Globals. */
struct request_list *rl_start = NULL;
struct request_list *rl_end = NULL;

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

        /* Find the module_var node for this variable. */
        if ((mv = module_var_find_by_name(var)) == NULL)
                return 0;

        printf("Request list add: id[%s] var[%s] args[%s]\n", id, var, args);

        /* Create request_list node... */
        n = malloc(sizeof(struct request_list));

        n->id = id;
        n->conn = conn;
        n->var = mv;
        n->args = args;
        n->remove = remove;

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
        cur->var->parent->clients--;

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
 * @brief Clear the request list.
 */
void request_list_clear(void)
{
        struct request_list *cur = rl_start;
        struct request_list *next;

        while (cur) {
                next = cur->next;

                freeif(cur->id);
                free(cur);
                
                cur = next;
        }
}
