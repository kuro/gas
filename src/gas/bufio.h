
/**
 * @file bufio.h
 * @brief bufio definition
 */

#ifndef GAS_BUFIO_H
#define GAS_BUFIO_H

#include <gas/tree.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @name buffer io */
/*@{*/

chunk* gas_read_buf (GASubyte* buf, GASunum limit, GASnum* offset);
GASnum gas_write_buf (chunk* self, GASubyte* buf);

GASnum gas_read_encoded_num_buf (GASubyte* buf, GASunum len, GASunum* result);
GASnum gas_write_encoded_num_buf (GASubyte* buf, GASunum value);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_BUFIO_H defined */

// vim: sw=4 fdm=marker
