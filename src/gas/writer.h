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

#ifndef GAS_WRITER_H
#define GAS_WRITER_H

#include <gas/context.h>
#include <gas/tree.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


typedef struct
{
    GAScontext* context;
    void *handle;
} GASwriter;

GASresult gas_write_encoded_num_writer (GASwriter *writer, GASunum value);
GASresult gas_write_writer (GASwriter *writer, GASchunk* self);

#ifdef __cplusplus
}
#endif

#endif /* GAS_WRITER_H defined */

/* vim: set sw=4 fdm=marker :*/
