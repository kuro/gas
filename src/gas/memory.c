/*
 * Copyright 2008 Blanton Black
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file memory.c
 */

#include "memory.h"

void* gas_default_alloc (unsigned int size)/*{{{*/
{
    return malloc(size);
}/*}}}*/
void* gas_default_realloc (void *ptr, unsigned int size)/*{{{*/
{
    return realloc(ptr, size);
}/*}}}*/
void gas_default_free (void *ptr)/*{{{*/
{
    free(ptr);
}/*}}}*/

GAS_MEMORY_ALLOC_CALLBACK   gas_alloc   = gas_default_alloc;
GAS_MEMORY_REALLOC_CALLBACK gas_realloc = gas_default_realloc;
GAS_MEMORY_FREE_CALLBACK    gas_free    = gas_default_free;

/**
 * @warning pool usage is not currently supported!
 */
GASresult gas_memory_initialize (/*{{{*/
    void *pool, int pool_len,
    GAS_MEMORY_ALLOC_CALLBACK   user_alloc,
    GAS_MEMORY_REALLOC_CALLBACK user_realloc,
    GAS_MEMORY_FREE_CALLBACK    user_free
    )
{
    if (pool != NULL && pool_len != 0) {
        // see warning above
        return GAS_ERR_INVALID_PARAM;
    } else if ((pool != NULL) ^ (pool_len != 0)) {
        return GAS_ERR_INVALID_PARAM;
    } else {
        if ((user_alloc == NULL) ||
            (user_realloc == NULL) ||
            (user_free == NULL)) {
            return GAS_ERR_INVALID_PARAM;
        }
        gas_alloc   = user_alloc;
        gas_realloc = user_realloc;
        gas_free    = user_free;
    }
    return GAS_OK;
}/*}}}*/

// vim: sw=4 fdm=marker
