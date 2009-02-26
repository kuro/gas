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
 * @file memory.h
 * @brief memory definition
 */

#ifndef GAS_MEMORY_H
#define GAS_MEMORY_H

#include <gas/types.h>

/**
 * @defgroup memory Memory
 * @brief Gas Memory Management
 */
/*@{*/

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif

typedef void* (*GAS_MEMORY_ALLOC_CALLBACK)   (unsigned int size,
                                              GASvoid* user_data);
typedef void* (*GAS_MEMORY_REALLOC_CALLBACK) (void *ptr, unsigned int size,
                                              GASvoid* user_data);
typedef void  (*GAS_MEMORY_FREE_CALLBACK)    (void *ptr, GASvoid* user_data);

GASresult gas_memory_initialize (
    GAS_MEMORY_ALLOC_CALLBACK   user_alloc,
    GAS_MEMORY_REALLOC_CALLBACK user_realloc,
    GAS_MEMORY_FREE_CALLBACK    user_free
    );

#if GAS_DEBUG_MEMORY || defined(DOXYGEN)
/**
 * @brief Get current memory usage from default allocator.
 */
GASunum gas_memory_usage (void);
#endif

extern GAS_MEMORY_ALLOC_CALLBACK   gas_alloc;
extern GAS_MEMORY_REALLOC_CALLBACK gas_realloc;
extern GAS_MEMORY_FREE_CALLBACK    gas_free;

#ifdef __cplusplus
} /* extern C */
#endif

/*@}*/

#endif  /* GAS_MEMORY_H defined */

/* vim: set sw=4 fdm=marker :*/
