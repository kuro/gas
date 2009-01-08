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
 * @file tree.c
 * @brief gas implementation
 */

/**
 * @mainpage
 *
 * @htmlinclude README.html
 *
 * Additional Topics:
 * - @ref parser
 */

#include <gas/tree.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int overwrite_attributes = GAS_TRUE;

GASchar* gas_error_string (GASresult result)/*{{{*/
{
    switch (result) {
    case GAS_OK:                  return "no error";
    case GAS_ERR_INVALID_PARAM:   return "invalid parameter";
    case GAS_ERR_FILE_NOT_FOUND:  return "file not found";
    case GAS_ERR_FILE_EOF:        return "end of file";
    case GAS_ERR_ATTR_NOT_FOUND:  return "attribute not found";
    case GAS_ERR_OUT_OF_RANGE:    return "value out of range";
    case GAS_ERR_MEMORY:          return "out of memory";
    case GAS_ERR_UNKNOWN:         return "unknown error";
    default: return (result > 0) ? "no error" : "invalid error code";
    }
}/*}}}*/

/** @name helper functions */
/*@{*/
/* gas_cmp() {{{*/
int gas_cmp (const GASubyte *a, GASunum a_len, const GASubyte *b, GASunum b_len)
{
    int result;
    unsigned int i = 0;

    GAS_CHECK_PARAM(a);
    GAS_CHECK_PARAM(b);

    while (1) {
        if (i == a_len) {
            result = (a_len == b_len) ? 0 : -1;
            break;
        }
        if (i == b_len) {
            result = (a_len == b_len) ? 0 : 1;
            break;
        }
        if (a[i] < b[i]) {
            result = -1;
            break;
        }
        if (a[i] > b[i]) {
            result = 1;
            break;
        }
        i++;
    }
    return result;
}
/*}}}*/

GASresult gas_hexdump_f (FILE* fs, const GASvoid *input, GASunum size)
{
    GASunum i, x, o;
    GASubyte *buf = (GASubyte*)input;
    GASchar characters[17];

    if (size == 0) {
        fprintf(fs, "%07lx: \n", 0x0l);
        return GAS_OK;
    }

    GAS_CHECK_PARAM(fs);
    GAS_CHECK_PARAM(input);

    characters[16] = '\0';

    o = 0;
    for (i = 0; i < (size >> 4); i++) {
        fprintf(fs, "%07lx: ", i << 4);
        for (x = 0; x < 8; x++) {
            fprintf(fs, "%02x%02x ", buf[o], buf[o+1]);
            characters[x << 1] = isprint(buf[o]) ? buf[o] : '.';
            characters[( x << 1 ) + 1] = isprint(buf[o+1]) ? buf[o+1] : '.';
            o += 2;
        }
        fprintf(fs, " %s", characters);
        fprintf(fs, "\n");
    }

    memset(characters, 0, 16);

    // finally
    fprintf(fs, "%07lx: ", (size >> 4) << 4);
    x = 0;
    for (i = 0; i < (size % 16); i++) {
        fprintf(fs, "%02x", buf[o]);
        characters[i] = isprint(buf[o]) ? buf[o] : '.';
        if (x) {
            fprintf(fs, " ");
        }
        x = ! x;
        o += 1;
    }
    for (; i < 16; i++) {
        fprintf(fs, "  ");
        if (x) {
            fprintf(fs, " ");
        }
        x = ! x;
    }
    fprintf(fs, " %s\n", characters);

    return GAS_OK;
}

GASresult gas_hexdump (const GASvoid *input, GASunum size)
{
    return gas_hexdump_f(stderr, input, size);
}

/*@}*/

/** @name helper functions and macros */
/*@{*/
/* gas_encoded_size() {{{*/
GASunum gas_encoded_size (GASunum value)
{
    int i, coded_length;
	int zero_count, zero_bytes;

    for (i = 1; 1; i++) {
        /*if (value < (unsigned int)pow(2, 7*i)-2) {*/
        if (value < ((1UL << (7UL*i))-1UL)) {
            break;
        }
    }
    coded_length = i;  /* not including header */
    /*printf("coded_length: %d\n", coded_length); */

    zero_count = coded_length - 1;
    zero_bytes = zero_count / 8;
    /*int zero_bits = zero_count % 8; */

    return coded_length + zero_bytes;
}
/*}}}*/
/* macro copy_to_field() {{{*/
#define copy_to_field(field)                                                \
    do {                                                                    \
        c->field##_size = field##_size;                                     \
        c->field = (GASubyte*)gas_realloc(c->field, field##_size + 1,       \
                                          c->user_data);                    \
        GAS_CHECK_MEM(c->field);                                            \
        memcpy(c->field, field, field##_size);                              \
        ((GASubyte*)c->field)[field##_size] = 0;                            \
    } while (0)
/*}}}*/
/* macro copy_to_attribute() {{{*/
#define copy_to_attribute(field)                                            \
    do {                                                                    \
        a->field##_size = field##_size;                                     \
        ctmp = (GASubyte*)gas_realloc(a->field, field##_size + 1,           \
                                      c->user_data);                        \
        GAS_CHECK_MEM(ctmp);                                                \
        a->field = ctmp;                                                    \
        memcpy(a->field, field, field##_size);                              \
        ((GASubyte*)a->field)[field##_size] = 0;                            \
    } while (0)
/*}}}*/
/*}@*/

/** @name cons/decons */
/*@{*/
/* gas_new() {{{*/
/**
 * @warning no way of reporting an error.
 */
GASresult gas_new (GASchunk** chunk, const GASvoid *id, GASunum id_size,
                   GASvoid *user_data)
{
    GASchunk *c;

    c = (GASchunk*)gas_alloc(sizeof(GASchunk), user_data);
    GAS_CHECK_MEM(c);

    memset(c, 0, sizeof(GASchunk));

    if (id) {
        c->id_size = id_size;
        c->id = (GASubyte*)gas_realloc(c->id, id_size + 1, user_data);
        GAS_CHECK_MEM(c->id);
        memcpy(c->id, id, id_size);
        ((GASubyte*)c->id)[id_size] = 0;
    }

    c->user_data = user_data;

    *chunk = c;

    return GAS_OK;
}
/*}}}*/
/* gas_new_named() {{{*/
GASresult gas_new_named (GASchunk**chunk, const char *id, GASvoid *user_data)
{
    return gas_new(chunk, id, strlen(id), user_data);
}
/*}}}*/
/* gas_destroy() {{{*/
/**
 * @brief Destroy a chunk tree.
 *
 * @note This does not release data for id, or the data contained in the
 * attributes, or the payload data.
 */
GASresult gas_destroy (GASchunk* c)
{
    GASunum i;
    GASresult result;

    GAS_CHECK_PARAM(c);

    gas_free(c->id, c->user_data);
    for (i = 0; i < c->nb_attributes; i++) {
        gas_free(c->attributes[i].key, c->user_data);
        gas_free(c->attributes[i].value, c->user_data);
    }
    gas_free(c->attributes, c->user_data);
    gas_free(c->payload, c->user_data);
    for (i = 0; i < c->nb_children; i++) {
        result = gas_destroy(c->children[i]);
#ifdef GAS_DEBUG
        if (result != GAS_OK) { return result; }
#endif
    }
    gas_free(c->children, c->user_data);
    gas_free(c, c->user_data);

    return GAS_OK;
}
/*}}}*/
/*@}*/

/** @name id access */
/*@{*/
/* gas_set_id() {{{*/
GASresult gas_set_id (GASchunk* c, const GASvoid *id, GASunum id_size)
{
    GAS_CHECK_PARAM(c);
    GAS_CHECK_PARAM(id);

    copy_to_field(id);
    return GAS_OK;
}
/*}}}*/
/* gas_id_size() {{{ */
GASunum gas_id_size (GASchunk* c)
{
    GAS_CHECK_PARAM(c);

    return c->id_size;
}
/*}}}*/
/* gas_get_id() {{{*/
/**
 * @return The number of bytes fetched.
 */
GASnum gas_get_id (GASchunk* c, GASvoid* id, GASunum limit)
{
    GAS_CHECK_PARAM(c);
    GAS_CHECK_PARAM(id);

    if (c->id_size < limit) {
        return GAS_ERR_INVALID_PARAM;
    }

    memcpy(((GASubyte*)id), c->id, c->id_size);
    return c->id_size;
}
/*}}}*/
/*@}*/

/** @name GASattribute access */
/*@{*/
/* gas_index_of_attribute() {{{*/
/**
 * @return signed index
 * @retval GAS_ERR_ATTR_NOT_FOUND failure, attribute not found
 */
GASnum gas_index_of_attribute (GASchunk* c, const GASvoid* key, GASunum key_size)
{
    GASunum i;
    GASattribute* a;

    GAS_CHECK_PARAM(c);
    GAS_CHECK_PARAM(key);

    for (i = 0; i < c->nb_attributes; i++ ) {
        a = &c->attributes[i];
        if (gas_cmp((GASubyte*)a->key, a->key_size,
                    (GASubyte*)key, key_size) == 0)
        {
            return i;
        }
    }
    return GAS_ERR_ATTR_NOT_FOUND;
}
/*}}}*/
/* gas_set_attribute() {{{*/
GASresult gas_set_attribute (GASchunk* c,
                           const GASvoid *key, GASunum key_size,
                           const GASvoid *value, GASunum value_size)
{
	GASattribute* tmp, *a;
    GASnum index;
    GASubyte *ctmp;

    GAS_CHECK_PARAM(c);
    GAS_CHECK_PARAM(key);
    GAS_CHECK_PARAM(value);

    index = gas_index_of_attribute(c, key, key_size);
    if (index >= 0 && overwrite_attributes) {
        /* found, replace */
        a = &c->attributes[index];
        copy_to_attribute(key);
        copy_to_attribute(value);
    } else {
        /* not found, append at end */
        c->nb_attributes++;

        tmp = (GASattribute*)gas_realloc(c->attributes,
                                     c->nb_attributes*sizeof(GASattribute), c->user_data);
        GAS_CHECK_MEM(tmp);
        c->attributes = tmp;

        a = &c->attributes[c->nb_attributes-1];
        a->key = NULL;
        a->value = NULL;
        copy_to_attribute(key);
        copy_to_attribute(value);
    }

    return GAS_OK;
}
/*}}}*/
/* gas_attribute_value_size() {{{*/
GASnum gas_attribute_value_size (GASchunk* c, GASunum index)
{
    GAS_CHECK_PARAM(c);

    if (index < c->nb_attributes) {
        return c->attributes[index].value_size;
    } else {
        return GAS_ERR_OUT_OF_RANGE;
    }
}
/*}}}*/
/* gas_get_attribute() {{{*/
/**
 * @note This method does not allocate or copy value data.
 * @return bytes fetched
 */
GASnum gas_get_attribute (GASchunk* c, GASunum index,
                          GASvoid* value, GASunum limit)
{
    GASattribute* a;

    GAS_CHECK_PARAM(c);
    GAS_CHECK_PARAM(value);

    a = &c->attributes[index];


    if (index >= c->nb_attributes) {
        return GAS_ERR_OUT_OF_RANGE;
    }

    if (a->value_size > limit) {
        return GAS_ERR_INVALID_PARAM;
    }

    a = &c->attributes[index];
    memcpy(((GASubyte*)value), a->value, a->value_size);
    return a->value_size;
}
/*}}}*/
/* gas_has_attribute() {{{*/
/**
 * @warning errors simply return false.
 */
GASbool gas_has_attribute (GASchunk* c, const GASvoid* key, GASunum key_size)
{

#ifdef GAS_DEBUG
    if (c == NULL) {
        fprintf(stderr, "gas error: %s @ %d: chunk was null\n",
                basename(__FILE__), __LINE__);
        return GAS_FALSE;
    }
    if (key == NULL) {
        fprintf(stderr, "gas error: %s @ %d: key was null\n",
                basename(__FILE__), __LINE__);
        return GAS_FALSE;
    }
#endif

    return gas_index_of_attribute(c, key, key_size) < 0 ? GAS_FALSE : GAS_TRUE;
}
/*}}}*/
GASresult gas_delete_attribute_at (GASchunk* c, GASunum index)/*{{{*/
{
    int trailing = 0;
    GASattribute *a = NULL;

    GAS_CHECK_PARAM(c);

    if (index >= c->nb_attributes) {
        return GAS_ERR_INVALID_PARAM;
    }
    a = &c->attributes[index];
    gas_free(a->value, c->user_data);
    gas_free(a->key, c->user_data);
    c->nb_attributes--;
    trailing = c->nb_attributes - index;
    if (trailing != 0) {
        memmove(&c->attributes[index], &c->attributes[index+1],
                trailing * sizeof(GASattribute));
    }
    return GAS_OK;
}/*}}}*/
/*@}*/

/** @name payload access */
/*@{*/
/* gas_set_payload() {{{*/
/**
 * @param payload When the payload is null, GASwriter will invoke a callback.
 */
GASresult gas_set_payload (GASchunk* c, const GASvoid *payload, GASunum payload_size)
{
    GAS_CHECK_PARAM(c);

    if (payload) {
        copy_to_field(payload);
    } else {
        c->payload_size = payload_size;
    }
    return GAS_OK;
}
/*}}}*/
/* gas_payload_size() {{{ */
/**
 * @warning no way of reporting an error.
 */
GASunum gas_payload_size (GASchunk* c)
{
    //GAS_CHECK_PARAM(c);
#ifdef GAS_DEBUG
    if (c == NULL) { return 0; }
#endif

    return c->payload_size;
}
/*}}}*/
/* gas_get_payload() {{{*/
/**
 * @return The number of bytes fetched.
 */
GASnum gas_get_payload (GASchunk* c, GASvoid* payload, GASunum limit)
{
    GAS_CHECK_PARAM(c);
    GAS_CHECK_PARAM(payload);

    if (c->payload_size < limit) {
        return GAS_ERR_INVALID_PARAM;
    }

    memcpy(((GASubyte*)payload), c->payload, c->payload_size);
    return c->payload_size;
}
/*}}}*/
/*@}*/

/** @name child access */
/*@{*/
/* gas_get_parent() {{{*/
/**
 * @warning no way of reporting an error.
 */
GASchunk* gas_get_parent(GASchunk* c)
{
#ifdef GAS_DEBUG
    if (c == NULL) { return NULL; }
#endif
    return c->parent;
}
/*}}}*/
/* gas_add_child() {{{*/
GASresult gas_add_child(GASchunk* parent, GASchunk* child)
{
    GASchunk** tmp;

    GAS_CHECK_PARAM(parent);
    GAS_CHECK_PARAM(child);

    parent->nb_children++;

    tmp = (GASchunk**)gas_realloc(parent->children,
                                  parent->nb_children * sizeof(GASchunk*), parent->user_data);
    GAS_CHECK_MEM(tmp);
    parent->children = tmp;

    parent->children[parent->nb_children - 1] = child;
    child->parent = parent;

    child->user_data = parent->user_data;

    return GAS_OK;
}
/*}}}*/
/* gas_nb_children() {{{*/
/**
 * @warning no way of reporting an error.
 */
GASunum gas_nb_children (GASchunk *c)
{
#ifdef GAS_DEBUG
    if (c == NULL) { return 0; }
#endif
    return c->nb_children;
}
/*}}}*/
/* gas_get_child_at() {{{*/
GASchunk* gas_get_child_at (GASchunk* c, GASunum index)
{
    if (index >= c->nb_children) {
        return NULL;
    }
    return c->children[index];
}
/*}}}*/
GASresult gas_delete_child_at (GASchunk* c, GASunum index)/*{{{*/
{
    int trailing = 0;

    GAS_CHECK_PARAM(c);

    if (index >= c->nb_children) {
        return GAS_ERR_INVALID_PARAM;
    }
    gas_destroy(c->children[index]);
    c->nb_children--;
    trailing = c->nb_children - index;
    if (trailing != 0) {
        memmove(&c->children[index], &c->children[index+1],
                trailing * sizeof(GASchunk*));
    }
    return GAS_OK;
}/*}}}*/
/*@}*/

/** @name management */
/*@{*/
/* gas_update() {{{*/
GASresult gas_update (GASchunk* c)
{
    GASresult result;
    GASunum i;

    GASunum sum;
    /*GASunum a, b;*/

    GAS_CHECK_PARAM(c);

    sum = 0;
    /* id*/
    sum += gas_encoded_size(c->id_size);
    sum += c->id_size;
    /* attributes */
    sum += gas_encoded_size(c->nb_attributes);
    for (i = 0; i < c->nb_attributes; i++) {
        sum += gas_encoded_size(c->attributes[i].key_size);
        sum += c->attributes[i].key_size;
        sum += gas_encoded_size(c->attributes[i].value_size);
        sum += c->attributes[i].value_size;
    }
    /* payload */
    sum += gas_encoded_size(c->payload_size);
    sum += c->payload_size;
    /* children */
    sum += gas_encoded_size(c->nb_children);
    for (i = 0; i < c->nb_children; i++) {
        GASchunk* child = c->children[i];
        result = gas_update(child);
#ifdef GAS_DEBUG
        if (result != GAS_OK) { return result; }
#endif
        sum += gas_encoded_size(child->size);
        sum += child->size;
    }

    /*printf("size: %ld\n", sum); */
    c->size = sum;
    /*fflush(stdout);*/

    return GAS_OK;
}
/*}}}*/
/* gas_total_size() {{{*/
/**
 * @brief Returns the total size of the chunk, including initial encoded size.
 * @warning The result is only valid after an update.
 * @warning no way of reporting an error.
 */
GASunum gas_total_size (GASchunk* c)
{
#ifdef GAS_DEBUG
    if (c == NULL) { return 0; }
#endif
    return c->size + gas_encoded_size(c->size);
}
/*}}}*/
/*@}*/


/* vim: set sw=4 fdm=marker : */
