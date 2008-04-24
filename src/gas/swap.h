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

#ifndef GAS_SWAP_H
#define GAS_SWAP_H

#include <gas/types.h>

GAS_INLINE float swapf(float fin);

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
      ((x & 0x000000ffU) << 24) |                                          \
      ((x & 0x0000ff00U) <<  8) |                                          \
      ((x & 0x00ff0000U) >>  8) |                                          \
      ((x & 0xff000000U) >> 24)                                            \
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

#ifndef GAS_INLINE
#error "GAS_INLINE not defined"
#endif

GAS_INLINE float swapf(float fin)
{
    uint32_t tmp = swap32(*(uint32_t*)&fin);
    return *(float*)&tmp;
}

#if GAS_BIG_ENDIAN
# define ntohs(x)       (x)
# define ntohl(x)       (x)
# define ntohf(x)       (x)
#else
# define ntohs(x) swap16(x)
# define ntohl(x) swap32(x)
# define ntohf(x)  swapf(x)
#endif

#define htons ntohs
#define htonl ntohl
#define htonf ntohf
#define htons ntohs
#define htonl ntohl

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @param Unconditional swap routine.
 *
 * @param buf array buffer to swap bytes
 * @param stride byte stride, 2 or 4 (short and long, respectively)
 * @param buffsize the total data length of the buffer
 *
 * @retval 0 success, otherwise failure
 */
int gas_swap (void *buf, size_t stride, size_t bufsize);

#ifdef __cplusplus
}
#endif


#endif /* GAS_SWAP_H defined */

/* vim: set sw=4 fdm=marker :*/
