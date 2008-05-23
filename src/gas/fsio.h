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
 * @file fsio.h
 * @brief fsio definition
 */

#include <gas/tree.h>
#include <stdio.h>

#ifndef GAS_FSIO_H
#define GAS_FSIO_H

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/**
 * @defgroup fsio File Stream IO
 * @ingroup io
 */
/*@{*/

GASresult gas_write_fs (FILE* fs, GASchunk* self);
GASresult gas_read_fs (FILE* fs, GASchunk **out);

GASresult gas_write_encoded_num_fs (FILE* fs, GASunum value);
GASresult gas_read_encoded_num_fs (FILE* fs, GASunum *value);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_FSIO_H defined */

/* vim: set sw=4 fdm=marker :*/
