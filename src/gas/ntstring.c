/*
 * Copyright 2008 Blanton Black
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file ntstring.c
 * @brief ntstring implementation
 *
 * @sa DUPLICATE_STRINGS
 */

#include "ntstring.h"

#include <string.h>

#if HAVE_STDIO_H
#include <stdio.h>
#endif

/**
 * @brief compile time option specifying whether or not to duplicate strings.
 *
 * @remarks The tree routines always allocate 1 extra byte, and set it to zero.
 * Consequently, it is not absolutely necessary to automatically duplicate the
 * string.  Instead, if the application guarantees that contained data consists
 * of clean strings, which it should, then the caller can call strdup() on the
 * null-terminated strings returned here.  Again, this is the application's
 * responsibility.  When in doubt, use the standard binary blob routines.
 */
#define DUPLICATE_STRINGS 0

GASchar* gas_sanitize (const GASubyte* str, GASunum len);

/** @name id access */
/*@{*/
/* gas_set_id_s() {{{*/
GASresult gas_set_id_s (GASchunk* c, const GASchar* id)
{
    return gas_set_id(c, id, strlen(id));
}
/*}}}*/
/* gas_get_id_s() {{{*/
/**
 * @brief Get the id as a string.
 */
#if DUPLICATE_STRINGS
/**
 * @return An allocated copy of the chunk id.
 * @attention Free the result when finished.
 */
#else
/**
 * @return A GASchar* that is guaranteed to be null terminated.
 * @warning Do not free the returned value.
 */
#endif
GASchar* gas_get_id_s (GASchunk* c)
{
#if DUPLICATE_STRINGS
    GASchar *retval;
    retval = (GASchar*)gas_alloc(c->id_size + 1);
    if (retval == NULL) {
        return NULL;
    }
    memcpy(retval, c->id, c->id_size);
    retval[c->id_size] = '\0';
    return retval;
#else
#if DEBUG
    if (c == NULL) {
        return NULL;
    }
#endif
    return (GASchar*)c->id;
#endif
}
/*}}}*/
/*@}*/


/** @name attribute access */
/*@{*/
/* gas_set_attribute_s() {{{*/
GASresult gas_set_attribute_s (GASchunk* c,
                               const GASchar *key,
                               const GASvoid *value, GASunum value_size)
{
    return gas_set_attribute(c, key, strlen(key), value, value_size);
}
/*}}}*/
/* gas_set_attribute_ss() {{{*/
GASresult gas_set_attribute_ss (GASchunk* c,
                                const GASchar *key, const GASchar *value)
{
    return gas_set_attribute(c, key, strlen(key), value, strlen(value));
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
 * @return status
 */
GASresult gas_get_attribute_s (GASchunk* c, const GASchar* key,
                               GASvoid* value, GASunum* len)
{
    return gas_get_attribute(c, key, strlen(key), value, len);
}
/*}}}*/
/* gas_get_attribute_ss() {{{*/
/**
 * @brief Get an attribute referected by string key as a string.
 */
#if DUPLICATE_STRINGS
/**
 * @return An allocated copy of the requested attribute.
 * @attention Free the result when finished.
 */
#else
/**
 * @return A GASchar* that is guaranteed to be null terminated.
 * @warning Do not free the returned value.
 */
#endif
GASchar* gas_get_attribute_ss (GASchunk* c, const GASchar* key)
{
    GASnum index;

#if DEBUG
    if (c == NULL) {
        return NULL;
    }
    if (key == NULL) {
        return NULL;
    }
#endif


    index = gas_index_of_attribute(c, key, strlen(key));
    if (index < 0) {
        return NULL;
    }

#if DUPLICATE_STRINGS
    GASnum status;
    GASunum len;
    GASchar *retval;
   
    len = gas_attribute_value_size(c, index);
    retval = (GASchar*)gas_alloc(len + 1);
    if (retval == NULL) {
        return NULL;
    }

    status = gas_get_attribute_at(c, index, retval, len);

    /**
     * @todo redundant checks
     */
    if (status < 0) {
        gas_free(retval);
        return NULL;
    }
    if (status != (GASnum)len) {
        gas_free(retval);
        return NULL;
    }

    retval[len] = '\0';

    return retval;
#else
    return (GASchar*)c->attributes[index].value;
#endif
}
/*}}}*/

GASbool gas_has_attribute_s (GASchunk* c, const GASchar* key)/*{{{*/
{
    return gas_has_attribute(c, key, strlen(key));
}/*}}}*/

GASresult gas_set_attribute_s_htonl(GASchunk* c, const GASchar* k, uint32_t v)/*{{{*/
{
    uint32_t v_net = htonl(v);
    return gas_set_attribute_s(c, k, &v_net, sizeof(v_net));
}/*}}}*/
GASresult gas_get_attribute_s_ntohl(GASchunk* c, const GASchar* k, uint32_t *v)/*{{{*/
{
    GASresult r;
    uint32_t v_net;
    GASunum len = sizeof(v_net);
    r = gas_get_attribute_s(c, k, &v_net, &len);
    if (r == GAS_OK) {
        *v = ntohl(v_net);
    }
    return r;
}/*}}}*/
GASresult gas_set_attribute_s_htons(GASchunk* c, const GASchar* k, uint16_t v)/*{{{*/
{
    uint16_t v_net = htons(v);
    return gas_set_attribute_s(c, k, &v_net, sizeof(v_net));
}/*}}}*/
GASresult gas_get_attribute_s_ntohs(GASchunk* c, const GASchar* k, uint16_t *v)/*{{{*/
{
    GASresult r;
    uint16_t v_net;
    GASunum len = sizeof(v_net);
    r = gas_get_attribute_s(c, k, &v_net, &len);
    if (r == GAS_OK) {
        *v = ntohs(v_net);
    }
    return GAS_OK;
}/*}}}*/

/*@}*/

/** @name payload access */
/*@{*/
/* gas_set_payload_s() {{{*/
GASresult gas_set_payload_s (GASchunk* c, const GASchar* payload)
{
    return gas_set_payload(c, payload, strlen(payload));
}
/*}}}*/
/* gas_get_payload_s() {{{*/
/**
 * @brief Fetch the payload as a null-terminated string.
 */
#if DUPLICATE_STRINGS
/**
 * @return An allocated copy of the chunk payload.
 * @attention Free the result when finished.
 */
#else
/**
 * @return A GASchar* that is guaranteed to be null terminated.
 * @warning Do not free the returned value.
 */
#endif
GASchar* gas_get_payload_s (GASchunk* c)
{

#if DEBUG
    if (c == NULL) {
        return NULL;
    }
#endif

#if DUPLICATE_STRINGS
    GASchar *retval;
    retval = (GASchar*)gas_alloc(c->payload_size + 1);
    if (retval == NULL) {
        return NULL;
    }
    memcpy(retval, c->payload, c->payload_size);
    retval[c->payload_size] = '\0';
    return retval;
#else
    return (GASchar*)c->payload;
#endif
}
/*}}}*/
/*@}*/

#if HAVE_FPRINTF

/** @name misc */
/*@{*/
/* gas_print(), for string based debugging only {{{ */

#define indent() for (level_iter=0;level_iter<level;level_iter++) printf("  ")

#include <ctype.h>

static GASbool gas_printable_all_or_nothing = GAS_TRUE;

GASchar* gas_sanitize (const GASubyte* str, GASunum len)
{
    GASbool printable;
    static GASchar san[1024 * 4];
    static GASchar hex[5];
    GASunum i, o = 0;

#if DEBUG
    if (str == NULL) {
        return NULL;
    }
#endif

    printable = GAS_TRUE;
    if (gas_printable_all_or_nothing) {
        for (i = 0; i < len; i++) {
            if (str[i] == 0 || ! isprint(str[i])) {
                printable = GAS_FALSE;
            }
        }
    }

    memset(san, 0, sizeof(san));
    for (i = 0; i < len; i++) {
        if (printable && (str[i] == 0 || isprint(str[i]))) {
            if (o+1 >= sizeof(san)) {
                return NULL;
            }
            san[o] = str[i];
            o+=1;
        } else {
            if (o+4 >= sizeof(san)) {
                return NULL;
            }
            sprintf(hex, "<%02x>", str[i] & 0xff);
            strncat(san, hex, sizeof(san));
            o+=4;
        }
    }
    return san;
}

GASresult gas_print (GASchunk* c)
{
    GASresult result;
    GASunum i;
    static int level = 0;
    static int level_iter;

    GAS_CHECK_PARAM(c);

    indent(); printf("---\n");
    /*indent(); printf("GASchunk of size = %ld\n", (unsigned long)c->size);*/
    indent(); printf("id[%ld]: \"%s\"\n", (unsigned long)c->id_size,
                     gas_sanitize(c->id, c->id_size));
    /*indent(); printf("%ld GASattribute(s):\n", (unsigned long)c->nb_attributes);*/
    for (i = 0; i < c->nb_attributes; i++) {
        indent();
        printf(
            "attr %ld of %ld [%ld,%ld]: \"%s\" ",
            i,
            c->nb_attributes,
            c->attributes[i].key_size,
            c->attributes[i].value_size,
            gas_sanitize(c->attributes[i].key, c->attributes[i].key_size)
            );
        printf(
            "-> \"%s\"\n",
            gas_sanitize(c->attributes[i].value, c->attributes[i].value_size)
            );
    }
    if (c->payload_size > 0) {
        if (c->payload == NULL) {
            indent(); printf("payload[%ld]: (undef)\n", c->payload_size);
        } else {
#if 0
            indent();
            printf("payload of size %ld:\n", (unsigned long)c->payload_size);
            printf("---\n%s\n^^^\n", (GASchar*)c->payload);
#else
            indent(); printf("payload[%ld]: \"%s\"\n",
                             c->payload_size,
                             gas_sanitize(c->payload, c->payload_size)
                            );
#endif
        }
    }

    level++;
    for (i = 0; i < c->nb_children; i++) {
        result = gas_print(c->children[i]);
#ifdef GAS_DEBUG
        if (result != GAS_OK) {
            return result;
        }
#endif
    }
    level--;

    return GAS_OK;
}

/* }}} */
/*@}*/

#endif

/* vim: set sw=4 fdm=marker :*/
