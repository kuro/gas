
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


/** @name fs io */
/*@{*/

void gas_write_fs (chunk* self, FILE* fs);
chunk* gas_read_fs (FILE* fs);

void gas_write_encoded_num_fs (FILE* fs, GASunum value);
GASunum gas_read_encoded_num_fs (FILE* fs);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_FSIO_H defined */

/* vim: set sw=4 fdm=marker :*/
