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

typedef void* (*GAS_MEMORY_ALLOC_CALLBACK)   (unsigned int size);
typedef void* (*GAS_MEMORY_REALLOC_CALLBACK) (void *ptr, unsigned int size);
typedef void  (*GAS_MEMORY_FREE_CALLBACK)    (void *ptr);

GASresult gas_memory_initialize (
    void *pool, int pool_len,
    GAS_MEMORY_ALLOC_CALLBACK   user_alloc,
    GAS_MEMORY_REALLOC_CALLBACK user_realloc,
    GAS_MEMORY_FREE_CALLBACK    user_free
    );

extern GAS_MEMORY_ALLOC_CALLBACK   gas_alloc;
extern GAS_MEMORY_REALLOC_CALLBACK gas_realloc;
extern GAS_MEMORY_FREE_CALLBACK    gas_free;

#endif  /* GAS_MEMORY_H defined */

// vim: sw=4 fdm=marker
