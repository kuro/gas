
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

#define swap32(x)                                                           \
    (                                                                       \
      ((x & 0x000000ffUL) << 24) |                                          \
      ((x & 0x0000ff00UL) <<  8) |                                          \
      ((x & 0x00ff0000UL) >>  8) |                                          \
      ((x & 0xff000000UL) >> 24)                                            \
    )

#if CMAKE_SIZEOF_VOID_P >= 8
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

inline float swapf (float fin)
{
    uint32_t tmp = swap32(*(uint32_t*)&fin);
    return *(float*)&tmp;
}

#if IS_BIG_ENDIAN
# define ntohf(x)      (x)
#else
# define ntohf(x) swapf(x)
#endif

#define htonf ntohf

#endif /* SWAP_H defined */

/* vim: set sw=4 fdm=marker :*/
