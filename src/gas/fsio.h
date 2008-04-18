
/**
 * @file fsio.h
 * @brief fsio definition
 */

#ifndef GAS_FSIO_H
#define GAS_FSIO_H


#include <gas/tree.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @defgroup fsio File Stream IO */
/*@{*/

GASresult gas_write_fs (FILE* fs, chunk* self);
GASresult gas_read_fs (FILE* fs, chunk **out);

GASresult gas_write_encoded_num_fs (FILE* fs, GASunum value);
GASresult gas_read_encoded_num_fs (FILE* fs, GASunum *value);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_FSIO_H defined */

/* vim: set sw=4 fdm=marker :*/
