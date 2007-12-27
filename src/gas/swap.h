
/**
 * @file swap.h
 * @brief swap definition
 */

#ifndef SWAP_H
#define SWAP_H

#define swap16(x)                                                           \
    (                                                                       \
      ((x & 0x00ffU) << 8) |                                                \
      ((x & 0xff00U) >> 8)                                                  \
    )

#if SIZEOF_VOID_P >= 8
#define swap32(x)                                                           \
    (                                                                       \
      ((x & 0x000000ffU) << 24) |                                          \
      ((x & 0x0000ff00U) <<  8) |                                          \
      ((x & 0x00ff0000U) >>  8) |                                          \
      ((x & 0xff000000U) >> 24)                                            \
    )
#else
#define swap32(x)                                                           \
    (                                                                       \
      ((x & 0x000000ffUL) << 24) |                                          \
      ((x & 0x0000ff00UL) <<  8) |                                          \
      ((x & 0x00ff0000UL) >>  8) |                                          \
      ((x & 0xff000000UL) >> 24)                                            \
    )
#endif

#if SIZEOF_VOID_P >= 8
#define swap64(x)                                                           \
    (                                                                       \
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
#include <stdlib.h>
#include <stdint.h>
inline float swapf (float fin)
{
    uint32_t tmp = swap32(*(uint32_t*)&fin);
    return *(float*)&tmp;
}

#if IS_BIG_ENDIAN
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


#endif /* SWAP_H defined */

/* vim: set sw=4 fdm=marker :*/
