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
 * @file types.h
 * @brief types definition
 */

#include <gas/gas.h>
#include <stdlib.h>

#ifndef GAS_TYPES_H
#define GAS_TYPES_H

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#else
#  ifdef MSVC
typedef short uint16_t;
typedef int uint32_t;
#  else
#    error "unable to determine system type information"
#  endif
#endif

#if GAS_USE_LONG_TYPES
typedef unsigned long int GASunum;
typedef          long int GASnum;
#else
typedef unsigned int GASunum;
typedef          int GASnum;
#endif
typedef unsigned char     GASubyte;
typedef          char     GASchar;
typedef int               GASenum;
typedef int               GASresult;
typedef void     GASvoid;

#ifdef __cplusplus
typedef bool GASbool;
#define GAS_FALSE false
#define GAS_TRUE  true
#else /* not __cplusplus */
typedef GASubyte GASbool;
enum
{
    GAS_FALSE,
    GAS_TRUE
};
#endif /* __cplusplus */

#endif /* GAS_TYPES_H defined */

/* vim: set sw=4 fdm=marker :*/
