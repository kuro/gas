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
 * @file swap.h
 * @brief swap definition
 */

#include "types.h"

#ifndef GAS_SWAP_H
#define GAS_SWAP_H

#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif

#ifdef HAVE_BYTESWAP_H

#  include <byteswap.h>
#  define swap16 bswap_16
#  define swap32 bswap_32
#  define swap64 bswap_64

#else

#define swap16(x)                                                           \
    (                                                                       \
      ((x & 0x00ffU) << 8) |                                                \
      ((x & 0xff00U) >> 8)                                                  \
    )

#define swap32(x)                                                           \
    ((uint32_t)                                                             \
      ((x & 0x000000ffU) << 24) |                                           \
      ((x & 0x0000ff00U) <<  8) |                                           \
      ((x & 0x00ff0000U) >>  8) |                                           \
      ((x & 0xff000000U) >> 24)                                             \
    )

#if GAS_SIZEOF_VOID_P >= 8
#define swap64(x)                                                           \
    ((uint64_t)                                                             \
      ((x & 0x00000000000000ffUL) << 56) |                                  \
      ((x & 0x000000000000ff00UL) << 40) |                                  \
      ((x & 0x0000000000ff0000UL) << 24) |                                  \
      ((x & 0x00000000ff000000UL) <<  8) |                                  \
      ((x & 0x000000ff00000000UL) >>  8) |                                  \
      ((x & 0x0000ff0000000000UL) >> 24) |                                  \
      ((x & 0x00ff000000000000UL) >> 40) |                                  \
      ((x & 0xff00000000000000UL) >> 56)                                    \
    )
#endif

#endif

#if !defined(HAVE_HTONL)
#  if GAS_BIG_ENDIAN
#   define ntohs(x)       (x)
#   define ntohl(x)       (x)
#  else
#   define ntohs(x) swap16(x)
#   define ntohl(x) swap32(x)
#  endif

#  define htons ntohs
#  define htonl ntohl
#endif

#if GAS_BIG_ENDIAN
# define ntohf(x)       (x)
#else
# define ntohf(x)  swapf(x)
#endif

#define htonf ntohf

#ifdef __cplusplus
extern "C"
{
#endif

GASresult gas_swap (GASvoid *buf, GASunum stride, GASunum bufsize);

#if defined(GAS_INLINE) && defined(__cplusplus)
GAS_INLINE float swapf (float fin)
{
    uint32_t tmp = swap32(*(uint32_t*)&fin);
    return *(float*)&tmp;
}
#else
float swapf (float fin);
#endif

#ifdef __cplusplus
}
#endif


#endif /* GAS_SWAP_H defined */

/* vim: set sw=4 fdm=marker :*/
