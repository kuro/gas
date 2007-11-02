
/**
 * @file gas.c
 * @brief gas implementation
 */

#include "gas.h"

#include <string.h>
#include <stdio.h>

#if UNIX
#include <assert.h>
#else
#define assert(expr) do {} while (0)
#endif

/* helper functions {{{*/
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
/*}}}*/
/* helper functions and macros {{{*/
/* encode_size() {{{*/
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
        c->field = malloc(field##_size + 1);                                \
        memcpy(c->field, field, field##_size);                              \
        ((GASubyte*)c->field)[field##_size] = 0;                             \
    } while (0)
/*}}}*/
/* macro copy_to_attribute() {{{*/
#define copy_to_attribute(field)                                            \
    do {                                                                    \
        a->field##_size = field##_size;                                     \
        a->field = malloc(field##_size + 1);                                \
        memcpy(a->field, field, field##_size);                              \
        ((GASubyte*)a->field)[field##_size] = 0;                             \
    } while (0)
/*}}}*/
/*}}}*/
/* cons/decons {{{*/
/* gas_new() {{{*/
chunk* gas_new (GASunum id_size, const void *id)
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
    return gas_new(strlen(id), id);
}
/*}}}*/
/* gas_destroy() {{{*/
/**
 * @brief Destroy a chunk tree.
 *
 * @note This does not release data for id, or the data contained in the
 * attributes, or the payload data.
 */
void gas_destroy (chunk* c)
{
    int i;
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
/*}}}*/
/* id access {{{*/
/* gas_set_id() {{{*/
void gas_set_id (chunk* c, GASunum id_size, const void *id)
{
    copy_to_field(id);
}
/*}}}*/
/* gas_get_id_s() {{{*/
char* gas_get_id_s (chunk* c)
{
    assert(((char*)c->id)[c->id_size] == '\0');
    return (char*)c->id;
}
/*}}}*/
/*}}}*/
/* attribute access {{{*/
/* gas_set_attribute() {{{*/
void gas_set_attribute (chunk* c,
                        GASunum key_size, const void *key,
                        GASunum value_size, const void *value)
{
	attribute* tmp, *a;

    c->nb_attributes++;

    tmp = realloc(c->attributes, c->nb_attributes*sizeof(attribute));
    assert(tmp);
    c->attributes = tmp;

    a = &c->attributes[c->nb_attributes-1];
    copy_to_attribute(key);
    copy_to_attribute(value);
}
/*}}}*/
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
/* gas_index_of_attribute() {{{*/
/**
 * @return signed index
 * @retval -1 failure, attribute not found
 */
GASunum gas_index_of_attribute (chunk* c, GASunum key_size, const void* key)
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
/* gas_has_attribute() {{{*/
int gas_has_attribute (chunk* c, GASunum key_size, void* key)
{
    return gas_index_of_attribute(c, key_size, key) == -1 ? 0 : 1;
}
/*}}}*/
/* gas_get_attribute() {{{*/
/**
 * @note This method does not allocate or copy value data.
 * @return request status
 * @retval GAS_FALSE failure
 * @retval GAS_TRUE success
 */
int gas_get_attribute (chunk* c,
                       GASunum key_size, const void* key,
                       GASunum* value_size, void* value)
{
    attribute* a;
    GASunum index = gas_index_of_attribute(c, key_size, key);

    if (index == -1) {
        if (value_size != NULL) {
            *value_size = 0;
        }
        /* *value = NULL;*/
        return GAS_FALSE;
    }

    a = &c->attributes[index];
    if (value_size != NULL) {
        *value_size = a->value_size;
    }
    /* *value = a->value;*/
    memcpy(value, a->value, a->value_size);
    return GAS_TRUE;
}
/*}}}*/
/* gas_get_attribute_s {{{*/
int gas_get_attribute_s (chunk* c,
                         const char* key,
                         GASunum* value_size, void* value)
{
    return gas_get_attribute(c, strlen(key), key, value_size, value);
}
/*}}}*/
/* gas_get_attribute_ss() {{{*/
/**
 * @note Caller is responsible for freeing result.
 */
char* gas_get_attribute_ss (chunk* c, const char* key)
{
    int status;
    GASunum value_size;
    GASunum key_size;
    char *value;
    GASunum index;

    key_size = strlen(key);
    
    index = gas_index_of_attribute(c, key_size, key);

    value = malloc(c->attributes[index].value_size + 1);
    status = gas_get_attribute(c, key_size, key, &value_size,(void*)value);
    value[value_size] = '\0';

    if (status == 0) {
        return NULL;
    }
    /* should already be null terminated */
    assert(value[value_size] == '\0');

    return value;
}
/*}}}*/
/*}}}*/
/* payload access {{{*/
/* gas_set_payload() {{{*/
void gas_set_payload (chunk* c, GASunum payload_size, const void *payload)
{
    copy_to_field(payload);
}
/*}}}*/
/* gas_set_payload_s() {{{*/
void gas_set_payload_s (chunk* c, const char* payload)
{
    gas_set_payload(c, strlen(payload), payload);
}
/*}}}*/
/* gas_get_payload_s() {{{*/
char* gas_get_payload_s (chunk* c)
{
    assert(((char*)c->payload)[c->payload_size] == '\0');
    return c->payload;
}
/*}}}*/
/*}}}*/
/* child access {{{*/
/* gas_add_child() {{{*/
void gas_add_child(chunk* parent, chunk* child)
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
/*}}}*/
/* management {{{*/
/* gas_update() {{{*/
/**
 * @todo finish and test
 */
void gas_update (chunk* c)
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

#if 0
    a = sum;
    //b = encoded_size(encoded_size(a) + encoded_size(b));
    b = encoded_size(encoded_size(a) + 10);
    sum = a + b;

    /* @todo what about summing own size? */
    /* this is just a best guess, and i don't like it */
    //sum += encoded_size(sum + 2);
#endif


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
/*}}}*/
/* misc {{{*/
/* gas_print(), for string based debugging only {{{ */

#define indent() for (level_iter=0;level_iter<level;level_iter++) printf("  ")

void gas_print (chunk* c)
{
    int i;
    static int level = 1;
    static int level_iter;

    indent(); printf("chunk of size = %ld\n", c->size);
    indent(); printf("id of size %ld -> \"%s\"\n", c->id_size, (char*)c->id);
    indent(); printf("%ld attribute(s):\n", c->nb_attributes);
    for (i = 0; i < c->nb_attributes; i++) {
        indent(); printf(
            "%d -- \"%s\" (%ld) -> \"%s\" (%ld)\n",
            i,
            (char*)c->attributes[i].key, c->attributes[i].key_size,
            (char*)c->attributes[i].value, c->attributes[i].value_size
            );
    }
    indent(); printf("payload of size %ld:\n", c->payload_size);
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
/*}}}*/

/* vim: set sw=4 fdm=marker fdl=1 : */
