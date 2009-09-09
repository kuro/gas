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
 * @file writer.h
 * @brief writer definition
 */

#include "context.h"
#include "tree.h"

#ifndef GAS_WRITER_H
#define GAS_WRITER_H

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif

/**
 * @defgroup writer Writer
 * @ingroup io
 */
/*@{*/

struct GASwriter;

typedef GASresult (*GAS_WRITE_PAYLOAD) (struct GASwriter* writer,
                                        GASchunk* c,
                                        unsigned int *bytes_written);

typedef struct GASwriter
{
    GAScontext* context;
    GASvoid *handle;

    GAS_WRITE_PAYLOAD on_write_payload;
} GASwriter;

GASresult gas_write_encoded_num_writer (GASwriter *writer, GASunum value);
GASresult gas_write_writer (GASwriter *writer, GASchunk* self);

GASresult gas_writer_new (
    GASwriter** writer,
    GAScontext* context,
    GASvoid* DEFAULT_NULL(handle),
    GASvoid* DEFAULT_NULL(user_data)
    );

GASresult gas_writer_destroy (GASwriter *w, GASvoid* DEFAULT_NULL(user_data));

GASresult gas_write (GASwriter* w, const char *resource, GASchunk *c);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_WRITER_H defined */

/* vim: set sw=4 fdm=marker :*/
