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
 * @file fdio.h
 * @brief fdio definition
 */

#include "tree.h"

#ifndef GAS_FDIO_H
#define GAS_FDIO_H

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/**
 * @defgroup fdio File Descriptor IO
 * @ingroup io
 */
/*@{*/


GASresult gas_write_fd (int fd, GASchunk* self);
GASresult gas_read_fd (int fd, GASchunk** out,
                       GASvoid* DEFAULT_NULL(user_data));

GASresult gas_write_encoded_num_fd (int fd, GASunum value);
GASresult gas_read_encoded_num_fd (int fd, GASunum* value);


/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_FDIO_H defined */

/* vim: set sw=4 fdm=marker :*/
