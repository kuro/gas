
/**
 * @file fdio.h
 * @brief fdio definition
 */

#ifndef GAS_FDIO_H
#define GAS_FDIO_H


#include <gas/gas.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @name fd io */
/*@{*/

void gas_write_fd (chunk* self, int fd);
chunk* gas_read_fd (int fd);

void gas_write_encoded_num_fd (int fd, GASunum value);
GASunum gas_read_encoded_num_fd (int fd);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_FDIO_H defined */

/* vim: set sw=4 fdm=marker :*/
