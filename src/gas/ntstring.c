
/**
 * @file ntstring.c
 * @brief ntstring implementation
 */

#include "ntstring.h"

#include <string.h>
#include <stdio.h>

#if UNIX
#include <assert.h>
#else
#define assert(expr) do {} while (0)
#endif

/** @name id access */
/*@{*/
/* gas_set_id_s() {{{*/
void gas_set_id_s (chunk* c, const char* id)
{
    gas_set_id(c, strlen(id), id);
}
/*}}}*/
/* gas_get_id_s() {{{*/
/**
 * Get the id as a string.
 *
 * Free the result when finished.
 */
char* gas_get_id_s (chunk* c)
{
    char *retval;
    retval = malloc(c->id_size + 1);
    assert(retval != NULL);
    if (retval == NULL) {
        return NULL;
    }
    memcpy(retval, c->id, c->id_size);
    retval[c->id_size] = '\0';
    return retval;
}
/*}}}*/
/*@}*/


/** @name attribute access */
/*@{*/
/* gas_set_attribute_s() {{{*/
void gas_set_attribute_s (chunk* c,
                          const char *key,
                          GASunum value_size, const void *value)
{
    gas_set_attribute(c, strlen(key), key, value_size, value);
}
/*}}}*/
/* gas_set_attribute_ss() {{{*/
void gas_set_attribute_ss(chunk* c, const char *key, const char *value)
{
    gas_set_attribute(c, strlen(key), key, strlen(value), value);
}
/*}}}*/
/* gas_get_attribute_s {{{*/
/**
 * @brief Very similar to gas_get_attribute(), but the key is assumed to be a null-terminated string.
 *
 * Basically, error handling is provided by gas_get_attribute.
 *
 * No memory is allocated.
 *
 * @returns status
 */
int gas_get_attribute_s (chunk* c, const char* key,
                         void* value, GASunum offset, GASunum limit)
{
    return gas_get_attribute (c,
                              gas_index_of_attribute(c, strlen(key), key),
                              value, offset, limit);
}
/*}}}*/
/* gas_get_attribute_ss() {{{*/
/**
 * @brief Get an attribute referected by string key as a string.
 * @note Caller is responsible for freeing result.
 *
 * Allocates memory.
 *
 * @returns An allocated copy of the requested attribute.
 */
char* gas_get_attribute_ss (chunk* c, const char* key)
{
    int status;
    GASunum len;
    GASnum index;
    char *retval;
   
    index = gas_index_of_attribute(c, strlen(key), key);
    if (index < 0) {
        return NULL;
    }

    len = gas_attribute_value_size(c, index);
    retval = malloc(len + 1);
    assert(retval != NULL);
    if (retval == NULL) {
        return NULL;
    }

    status = gas_get_attribute(c, index, retval, 0, len);
    if (status != 0) {
        free(retval);
        return NULL;
    }

    retval[len] = '\0';

    return retval;
}
/*}}}*/
/*@}*/

/** @name payload access */
/*@{*/
/* gas_set_payload_s() {{{*/
void gas_set_payload_s (chunk* c, const char* payload)
{
    gas_set_payload(c, strlen(payload), payload);
}
/*}}}*/
/* gas_get_payload_s() {{{*/
/**
 * @returns An allocated copy of the payload.
 */
char* gas_get_payload_s (chunk* c)
{
    char *retval;
    retval = malloc(c->payload_size + 1);
    assert(retval != NULL);
    if (retval == NULL) {
        return NULL;
    }
    memcpy(retval, c->payload, c->payload_size);
    retval[c->payload_size] = '\0';
    return retval;
}
/*}}}*/
/*@}*/

/** @name misc */
/*@{*/
/* gas_print(), for string based debugging only {{{ */

#define indent() for (level_iter=0;level_iter<level;level_iter++) printf("  ")

void gas_print (chunk* c)
{
    int i;
    static int level = 1;
    static int level_iter;

    if (c == NULL) {
        return;
    }

    indent(); printf("chunk of size = %ld\n", (unsigned long)c->size);
    indent(); printf("id of size %ld -> \"%s\"\n", (unsigned long)c->id_size, (char*)c->id);
    indent(); printf("%ld attribute(s):\n", (unsigned long)c->nb_attributes);
    for (i = 0; i < c->nb_attributes; i++) {
        indent(); printf(
            "%d -- \"%s\" (%ld) -> \"%s\" (%ld)\n",
            i,
            (char*)c->attributes[i].key, (unsigned long)c->attributes[i].key_size,
            (char*)c->attributes[i].value, (unsigned long)c->attributes[i].value_size
            );
    }
    indent(); printf("payload of size %ld:\n", (unsigned long)c->payload_size);
    if (c->payload_size > 0) {
        printf("---\n%s\n^^^\n", (char*)c->payload);
    }

    level++;
    for (i = 0; i < c->nb_children; i++) {
        gas_print(c->children[i]);
    }
    level--;
}

/* }}} */
/*@}*/

/* vim: set sw=4 fdm=marker :*/
