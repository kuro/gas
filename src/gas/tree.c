
/**
 * @file gas.c
 * @brief gas implementation
 */

/**
 * @mainpage
 *
 * Topics:
 * - @ref parser
 */

#include <gas/gas.h>

#include <string.h>
#include <stdio.h>

#if HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(expr) do {} while (0)
#endif


static int overwrite_attributes = GAS_TRUE;

/** @name helper functions */
/*@{*/
/* gas_cmp() {{{*/
int gas_cmp (GASunum a_len, const GASubyte *a, GASunum b_len, const GASubyte *b)
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
        if (value < ((1L << (7L*i))-1L)) {
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
        c->field = realloc(c->field, field##_size + 1);                     \
        memcpy(c->field, field, field##_size);                              \
        ((GASubyte*)c->field)[field##_size] = 0;                            \
    } while (0)
/*}}}*/
/* macro copy_to_attribute() {{{*/
#define copy_to_attribute(field)                                            \
    do {                                                                    \
        a->field##_size = field##_size;                                     \
        ctmp = realloc(a->field, field##_size + 1);                         \
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
chunk* gas_new (const GASvoid *id, GASunum id_size)
{
    chunk *c;

    c = malloc(sizeof(chunk));
    assert(c != NULL);

    memset(c, 0, sizeof(chunk));

    if (id) {
        copy_to_field(id);
    }

    return c;
}
/*}}}*/
/* gas_new_named() {{{*/
chunk* gas_new_named (const char *id)
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
GASvoid gas_destroy (chunk* c)
{
    int i;

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
GASvoid gas_set_id (chunk* c, const GASvoid *id, GASunum id_size)
{
    copy_to_field(id);
}
/*}}}*/
/* gas_id_size() {{{ */
GASunum gas_id_size (chunk* c)
{
    return c->id_size;
}
/*}}}*/
#define min(a, b) (a<b?a:b)
/* gas_get_id() {{{*/
/**
 * @returns The number of bytes remaining.
 * @todo add sanity checking
 */
GASnum gas_get_id (chunk* c, GASvoid* id, GASunum limit)
{
    GASunum count;

    count = min(limit, c->id_size);
    memcpy(((GASubyte*)id), c->id, count);
    return c->id_size - limit;
}
/*}}}*/
/*@}*/

/** @name attribute access */
/*@{*/
/* gas_index_of_attribute() {{{*/
/**
 * @return signed index
 * @retval -1 failure, attribute not found
 */
GASnum gas_index_of_attribute (chunk* c, const GASvoid* key, GASunum key_size)
{
    GASunum i;
    attribute* a;
    for (i = 0; i < c->nb_attributes; i++ ) {
        a = &c->attributes[i];
        if (gas_cmp(a->key_size, a->key, key_size, key) == 0) {
            return i;
        }
    }
    return -1;
}
/*}}}*/
/* gas_set_attribute() {{{*/
/**
 * @todo check for existing attribute first!
 */
GASvoid gas_set_attribute (chunk* c,
                           const GASvoid *key, GASunum key_size,
                           const GASvoid *value, GASunum value_size)
{
	attribute* tmp, *a;
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

        tmp = realloc(c->attributes, c->nb_attributes*sizeof(attribute));
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
GASnum gas_attribute_value_size (chunk* c, GASunum index)
{
    if (index < c->nb_attributes) {
        return c->attributes[index].value_size;
    } else {
        return -1;
    }
}
/*}}}*/
/* gas_get_attribute() {{{*/
/**
 * @note This method does not allocate or copy value data.
 * @returns bytes remaining
 */
GASnum gas_get_attribute (chunk* c, GASunum index,
                          GASvoid* value, GASunum limit)
{
    attribute* a;
    GASunum count;

    if (index >= c->nb_attributes) {
        return -1;
    }

    a = &c->attributes[index];
    count = min(limit, a->value_size);
    memcpy(((GASubyte*)value), a->value, count);
    return a->value_size - limit;
}
/*}}}*/
/* gas_has_attribute() {{{*/
GASbool gas_has_attribute (chunk* c, GASvoid* key, GASunum key_size)
{
    return gas_index_of_attribute(c, key, key_size) == -1 ? 0 : 1;
}
/*}}}*/
/*@}*/

/** @name payload access */
/*@{*/
/* gas_set_payload() {{{*/
GASvoid gas_set_payload (chunk* c, const GASvoid *payload, GASunum payload_size)
{
    copy_to_field(payload);
}
/*}}}*/
/* gas_payload_size() {{{ */
GASunum gas_payload_size (chunk* c)
{
    return c->payload_size;
}
/*}}}*/
/* gas_get_payload() {{{*/
/**
 * @returns The number of bytes remaining.
 * @todo add sanity checking
 */
GASunum gas_get_payload (chunk* c, GASvoid* payload, GASunum limit)
{
    GASunum count;

    count = min(limit, c->payload_size);
    memcpy(((GASubyte*)payload), c->payload, count);
    return c->payload_size - limit;
}
/*}}}*/
/*@}*/

/** @name child access */
/*@{*/
/* gas_get_parent() {{{*/
chunk* gas_get_parent(chunk* c)
{
    return c->parent;
}
/*}}}*/
/* gas_add_child() {{{*/
GASvoid gas_add_child(chunk* parent, chunk* child)
{
    chunk** tmp;

    parent->nb_children++;

    tmp = realloc(parent->children, parent->nb_children*sizeof(chunk*));
    assert(tmp);
    parent->children = tmp;

    parent->children[parent->nb_children - 1] = child;
    child->parent = parent;
}
/*}}}*/
/* gas_nb_children() {{{*/
GASunum gas_nb_children (chunk *c)
{
    return c->nb_children;
}
/*}}}*/
/* gas_get_child_at() {{{*/
chunk* gas_get_child_at (chunk* c, GASunum index)
{
    if (index >= c->nb_children) {
        return NULL;
    }
    return c->children[index];
}
/*}}}*/
/*@}*/

/** @name management */
/*@{*/
/* gas_update() {{{*/
GASvoid gas_update (chunk* c)
{
    int i;

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
        chunk* child = c->children[i];
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
GASunum gas_total_size (chunk* c)
{
    return c->size + encoded_size(c->size);
}
/*}}}*/
/*@}*/


/* vim: set sw=4 fdm=marker : */
