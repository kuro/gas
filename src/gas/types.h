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

#include "gas.h"
#include <stdlib.h>

#ifndef GAS_TYPES_H
#define GAS_TYPES_H

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#else

typedef          char   int8_t;
typedef unsigned char  uint8_t;

   /* 16 bit type */
#  if GAS_SIZEOF_SHORT_INT == 2
     typedef          short  int16_t;
     typedef unsigned short uint16_t;
#  else
#    error "unable to determine 16 bit type"
#  endif

   /* 32 bit type */
#  if GAS_SIZEOF_INT == 4
     typedef          int  int32_t;
     typedef unsigned int uint32_t;
#  elif GAS_SIZEOF_LONG_INT == 4
     typedef          long int  int32_t;
     typedef unsigned long int uint32_t;
#  else
#    error "unable to determine 32 bit type"
#  endif

#endif  /* HAVE_STDINT_H */

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
