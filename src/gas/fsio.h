
/**
 * @file fsio.h
 * @brief fsio definition
 */

#ifndef FSIO_H
#define FSIO_H


#include <gas/gas.h>
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

#endif /* FSIO_H defined */

/* vim: set sw=4 fdm=marker :*/
