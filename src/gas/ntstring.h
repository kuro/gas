
/**
 * @file ntstring.h
 * @brief ntstring definition
 */

#ifndef NTSTRING_H
#define NTSTRING_H 

#include "gas.h"

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

void gas_set_id_s (chunk* c, const char* id);
char* gas_get_id_s (chunk* c);

void gas_set_attribute_s (chunk* c,
                          const char *key,
                          GASunum value_size, const void *value);
void gas_set_attribute_ss(chunk* c, const char *key, const char *value);

int gas_get_attribute_s (chunk* c, const char* key,
                         void* value, GASunum offset, GASunum limit);
char* gas_get_attribute_ss (chunk* c, const char* key);

void gas_set_payload_s (chunk* c, const char* payload);
char* gas_get_payload_s (chunk* c);



/**
 * @brief Print out the tree, for debugging only.
 *
 * @warning This treats EVERYTHING as null terminated strings.  Only call
 * this when ids, attributes, and payloads are all strings.
 */
void gas_print (chunk* c);

/*@}*/

#endif /* NTSTRING_H defined */

/* vim: set sw=4 fdm=marker :*/
