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

#if HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(expr) do {} while (0)
#endif


static int overwrite_attributes = GAS_TRUE;

GASunum encoded_size (GASunum value);

GASchar* gas_error_string (GASresult result)
{
    switch (result) {
    case GAS_OK:                 return "no error";
    case GAS_ERR_INVALID_PARAM:  return "invalid parameter";
    case GAS_ERR_FILE_NOT_FOUND: return "file not found";
    case GAS_ERR_FILE_EOF:       return "end of file";
    case GAS_ERR_ATTR_NOT_FOUND: return "attribute not found";
    case GAS_ERR_OUT_OF_RANGE:   return "value out of range";
    case GAS_ERR_UNKNOWN:        return "unknown error";
    default: return (result > 0) ? "no error" : "invalid error code";
    }
}

/** @name helper functions */
/*@{*/
/* gas_cmp() {{{*/
int gas_cmp (const GASubyte *a, GASunum a_len, const GASubyte *b, GASunum b_len)
{
    int result;
    unsigned int i = 0;
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

void gas_hexdump_f (FILE* fs, GASvoid *input, GASunum size)
{
    GASunum i, x, o;
    GASubyte *buf = (GASubyte*)input;
    GASchar characters[17];
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
}

void gas_hexdump (GASvoid *input, GASunum size)
{
    gas_hexdump_f(stderr, input, size);
}

/*@}*/

/** @name helper functions and macros */
/*@{*/
/* encoded_size() {{{*/
GASunum encoded_size (GASunum value)
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
        c->field = (GASubyte*)realloc(c->field, field##_size + 1);          \
        memcpy(c->field, field, field##_size);                              \
        ((GASubyte*)c->field)[field##_size] = 0;                            \
    } while (0)
/*}}}*/
/* macro copy_to_attribute() {{{*/
#define copy_to_attribute(field)                                            \
    do {                                                                    \
        a->field##_size = field##_size;                                     \
        ctmp = (GASubyte*)realloc(a->field, field##_size + 1);              \
        assert(ctmp != NULL);                                               \
        a->field = ctmp;                                                    \
        memcpy(a->field, field, field##_size);                              \
        ((GASubyte*)a->field)[field##_size] = 0;                            \
    } while (0)
/*}}}*/
/*}@*/

/** @name cons/decons */
/*@{*/
/* gas_new() {{{*/
GASchunk* gas_new (const GASvoid *id, GASunum id_size)
{
    GASchunk *c;

    c = (GASchunk*)malloc(sizeof(GASchunk));
    assert(c != NULL);
    memset(c, 0, sizeof(GASchunk));

    if (id) {
        copy_to_field(id);
    }

    return c;
}
/*}}}*/
/* gas_new_named() {{{*/
GASchunk* gas_new_named (const char *id)
{
    return gas_new(id, strlen(id));
}
/*}}}*/
/* gas_destroy() {{{*/
/**
 * @brief Destroy a chunk tree.
 *
 * @note This does not release data for id, or the data contained in the
 * attributes, or the payload data.
 */
GASvoid gas_destroy (GASchunk* c)
{
    GASunum i;

    if (c == NULL) {
        return;
    }

    free(c->id);
    for (i = 0; i < c->nb_attributes; i++) {
        free(c->attributes[i].key);
        free(c->attributes[i].value);
    }
    free(c->attributes);
    free(c->payload);
    for (i = 0; i < c->nb_children; i++) {
        gas_destroy(c->children[i]);
    }
    free(c->children);
    free(c);
}
/*}}}*/
/*@}*/

/** @name id access */
/*@{*/
/* gas_set_id() {{{*/
GASvoid gas_set_id (GASchunk* c, const GASvoid *id, GASunum id_size)
{
    copy_to_field(id);
}
/*}}}*/
/* gas_id_size() {{{ */
GASunum gas_id_size (GASchunk* c)
{
    return c->id_size;
}
/*}}}*/
#define min(a, b) (a<b?a:b)
/* gas_get_id() {{{*/
/**
 * @return The number of bytes fetched.
 */
GASnum gas_get_id (GASchunk* c, GASvoid* id, GASunum limit)
{
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
GASvoid gas_set_attribute (GASchunk* c,
                           const GASvoid *key, GASunum key_size,
                           const GASvoid *value, GASunum value_size)
{
	GASattribute* tmp, *a;
    GASnum index;
    GASubyte *ctmp;

    index = gas_index_of_attribute(c, key, key_size);
    if (index >= 0 && overwrite_attributes) {
        /* found, replace */
        a = &c->attributes[index];
        copy_to_attribute(key);
        copy_to_attribute(value);
    } else {
        /* not found, append at end */
        c->nb_attributes++;

        tmp = (GASattribute*)realloc(c->attributes,
                                  c->nb_attributes*sizeof(GASattribute));
        assert(tmp);
        c->attributes = tmp;

        a = &c->attributes[c->nb_attributes-1];
        a->key = NULL;
        a->value = NULL;
        copy_to_attribute(key);
        copy_to_attribute(value);
    }

}
/*}}}*/
/* gas_attribute_value_size() {{{*/
GASnum gas_attribute_value_size (GASchunk* c, GASunum index)
{
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
GASbool gas_has_attribute (GASchunk* c, GASvoid* key, GASunum key_size)
{
    return gas_index_of_attribute(c, key, key_size) == -1 ? 0 : 1;
}
/*}}}*/
GASnum gas_delete_attribute_at (GASchunk* c, GASunum index)
{
    int trailing = 0;
    GASattribute *a = NULL;
    if (index >= c->nb_attributes) {
        return GAS_ERR_INVALID_PARAM;
    }
    a = &c->attributes[index];
    free(a->value);
    free(a->key);
    c->nb_attributes--;
    trailing = c->nb_attributes - index;
    if (trailing != 0) {
        memmove(&c->attributes[index], &c->attributes[index+1],
                trailing * sizeof(GASattribute));
    }
    return GAS_OK;
}
/*@}*/

/** @name payload access */
/*@{*/
/* gas_set_payload() {{{*/
GASvoid gas_set_payload (GASchunk* c, const GASvoid *payload, GASunum payload_size)
{
    copy_to_field(payload);
}
/*}}}*/
/* gas_payload_size() {{{ */
GASunum gas_payload_size (GASchunk* c)
{
    return c->payload_size;
}
/*}}}*/
/* gas_get_payload() {{{*/
/**
 * @return The number of bytes fetched.
 */
GASnum gas_get_payload (GASchunk* c, GASvoid* payload, GASunum limit)
{
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
GASchunk* gas_get_parent(GASchunk* c)
{
    return c->parent;
}
/*}}}*/
/* gas_add_child() {{{*/
GASvoid gas_add_child(GASchunk* parent, GASchunk* child)
{
    GASchunk** tmp;

    parent->nb_children++;

    tmp = (GASchunk**)realloc(parent->children,parent->nb_children*sizeof(GASchunk*));
    assert(tmp);
    parent->children = tmp;

    parent->children[parent->nb_children - 1] = child;
    child->parent = parent;
}
/*}}}*/
/* gas_nb_children() {{{*/
GASunum gas_nb_children (GASchunk *c)
{
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
GASnum gas_delete_child_at (GASchunk* c, GASunum index)
{
    int trailing = 0;
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
}
/*@}*/

/** @name management */
/*@{*/
/* gas_update() {{{*/
GASvoid gas_update (GASchunk* c)
{
    GASunum i;

    GASunum sum;
    /*GASunum a, b;*/

    sum = 0;
    /* id*/
    sum += encoded_size(c->id_size);
    sum += c->id_size;
    /* attributes */
    sum += encoded_size(c->nb_attributes);
    for (i = 0; i < c->nb_attributes; i++) {
        sum += encoded_size(c->attributes[i].key_size);
        sum += c->attributes[i].key_size;
        sum += encoded_size(c->attributes[i].value_size);
        sum += c->attributes[i].value_size;
    }
    /* payload */
    sum += encoded_size(c->payload_size);
    sum += c->payload_size;
    /* children */
    sum += encoded_size(c->nb_children);
    for (i = 0; i < c->nb_children; i++) {
        GASchunk* child = c->children[i];
        gas_update(child);
        sum += encoded_size(child->size);
        sum += child->size;
    }

    /*printf("size: %ld\n", sum); */
    c->size = sum;
    /*fflush(stdout);*/
}
/*}}}*/
/* gas_total_size() {{{*/
/**
 * @brief Returns the total size of the chunk, including initial encoded size.
 * @warning The result is only valid after an update.
 */
GASunum gas_total_size (GASchunk* c)
{
    return c->size + encoded_size(c->size);
}
/*}}}*/
/*@}*/


/* vim: set sw=4 fdm=marker : */
