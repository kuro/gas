
/**
 * @file fdio.h
 * @brief fdio definition
 */

#ifndef FDIO_H
#define FDIO_H


#include <gas/gas.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @name fd io */
/*@{*/

void gas_write_fd (chunk* self, int fd);
void gas_write_encoded_num_fd (int fd, GASunum value);
chunk* gas_read_fd (int fd);

void gas_write_encoded_num_fd (int fd, GASunum value);
GASunum gas_read_encoded_num_fd (int fd);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* FDIO_H defined */

/* vim: set sw=4 fdm=marker :*/
