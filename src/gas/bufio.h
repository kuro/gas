
/**
 * @file bufio.h
 * @brief bufio definition
 */

#ifndef BUFIO_H
#define BUFIO_H

#include <gas/gas.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @name buffer io */
/*@{*/
GASnum gas_buf_write_encoded_num (GASubyte* buf, GASunum value);
GASnum gas_buf_write (chunk* self, GASubyte* buf);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* BUFIO_H defined */

// vim: sw=4 fdm=marker
