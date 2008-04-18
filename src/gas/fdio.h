
/**
 * @file fdio.h
 * @brief fdio definition
 */

#ifndef GAS_FDIO_H
#define GAS_FDIO_H


#include <gas/tree.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @defgroup fdio File Descriptor IO */
/*@{*/


GASresult gas_write_fd (int fd, chunk* self);
GASresult gas_read_fd (int fd, chunk** out);

GASresult gas_write_encoded_num_fd (int fd, GASunum value);
GASresult gas_read_encoded_num_fd (int fd, GASunum* value);


/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_FDIO_H defined */

/* vim: set sw=4 fdm=marker :*/
