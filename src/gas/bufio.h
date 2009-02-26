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
 * @file bufio.h
 * @brief bufio definition
 */

#include <gas/tree.h>

#ifndef GAS_BUFIO_H
#define GAS_BUFIO_H

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/**
 * @defgroup bufio Buffer IO
 * @ingroup io
 */
/*@{*/

GASnum gas_read_buf (GASubyte* buf, GASunum limit, GASchunk** out,
                     GASvoid* DEFAULT_NULL(user_data));
GASnum gas_read_bufn (GASubyte* buf, GASunum limit, GASchunk** out,
                      GASvoid* DEFAULT_NULL(user_data));
GASnum gas_write_buf (GASubyte* buf, GASunum limit, GASchunk* self);

GASnum gas_read_encoded_num_buf (GASubyte* buf, GASunum limit, GASunum* result);
GASnum gas_write_encoded_num_buf (GASubyte* buf, GASunum limit, GASunum value);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_BUFIO_H defined */

/* vim: set sw=4 fdm=marker :*/
