
/**
 * @file types.h
 * @brief types definition
 */

#ifndef GAS_TYPES_H
#define GAS_TYPES_H

#include <gas/gas.h>

#include <stdlib.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#ifdef MSVC
typedef short uint16_t;
typedef int uint32_t;
#endif
#endif

typedef unsigned long int GASunum;
typedef          long int GASnum;
typedef unsigned char     GASubyte;
typedef          char     GASchar;
typedef int               GASenum;
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
