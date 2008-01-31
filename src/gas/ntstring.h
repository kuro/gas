
/**
 * @file ntstring.h
 * @brief ntstring definition
 */

#ifndef GAS_NTSTRING_H
#define GAS_NTSTRING_H 

#include <gas/gas.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif

/**
 * @defgroup ntstring
 * @brief null-terminated string support.
 *
 * The primary gas functions know nothing about data types.  The ntstring set
 * of commands provide convenient null terminated string based wrappers.  In
 * order to use these functions, it is the application's responsibility to
 * place strings in the gas containers.  That is to say, Gas does not enforce
 * consistency.
 */
/*@{*/

GASvoid gas_set_id_s (chunk* c, const GASchar* id);
GASchar* gas_get_id_s (chunk* c);

GASvoid gas_set_attribute_s (chunk* c,
                             const GASchar *key,
                             const GASvoid *value, GASunum value_size);
GASvoid gas_set_attribute_ss(chunk* c, const GASchar *key, const GASchar *value);

GASnum gas_get_attribute_s (chunk* c, const GASchar* key,
                         GASvoid* value, GASunum limit);
GASchar* gas_get_attribute_ss (chunk* c, const GASchar* key);

GASvoid gas_set_payload_s (chunk* c, const GASchar* payload);
GASchar* gas_get_payload_s (chunk* c);



/**
 * @brief Print out the tree, for debugging only.
 *
 * @warning This treats EVERYTHING as null terminated strings.  Only call
 * this when ids, attributes, and payloads are all strings.
 */
GASvoid gas_print (chunk* c);

/*@}*/

#ifdef __cplusplus
}
/*}*/
#endif
#endif /* GAS_NTSTRING_H defined */

/* vim: set sw=4 fdm=marker :*/
