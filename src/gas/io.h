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
 * @file io.h
 * @brief io definition
 */


#ifndef GAS_IO_H
#define GAS_IO_H

#include <gas/parser.h>
#include <gas/writer.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif

/**
 * @defgroup fullio Full IO
 * @ingroup io
 */
/*@{*/

typedef struct
{
    GASparser* parser;
    GASwriter* writer;
} GASio;

GASresult gas_io_new (
    GASio** io,
    GAScontext* context,
    GASvoid* DEFAULT_NULL(handle),
    GASvoid* DEFAULT_NULL(user_data)
    );

GASresult gas_io_destroy (GASio *io, GASvoid* DEFAULT_NULL(user_data));

GASresult gas_io_set_handle (GASio* io, GASvoid* handle);

GASresult gas_read_io (GASio* io, GASchunk** out, GASvoid* user_data);
GASresult gas_write_io (GASio* io, GASchunk* c);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_IO_H defined */


// vim: sw=4 fdm=marker
