
/**
 * @file gas.c
 * @brief gas implementation
 */

#include "gas.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#ifndef _MSC_VER
#include <unistd.h>
#include <linux/types.h>
#else
#include <basetsd.h>
typedef char uint8_t;
typedef long ssize_t;
#endif

/* function prototypes {{{*/
void gas_print (chunk* c);
/*}}}*/
/* helper functions and macros {{{*/
size_t encoded_size (size_t value)
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

#define copy_to_field(field)                                                \
    do {                                                                    \
        c->field##_size = field##_size;                                     \
        c->field = malloc(field##_size + 1);                                \
        memcpy(c->field, field, field##_size);                              \
        ((uint8_t*)c->field)[field##_size] = 0;                             \
    } while (0)
/*}}}*/

/* cons/decons {{{*/
chunk* gas_new (size_t id_size, const void *id)
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


chunk* gas_new_named (const char *id)
{
    return gas_new(strlen(id), id);
}


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
/* access {{{*/

void gas_set_id (chunk* c, size_t id_size, const void *id)
{
    copy_to_field(id);
}

char* gas_get_id_as_string (chunk* c)
{
    assert(((char*)c->id)[c->id_size] == '\0');
    return (char*)c->id;
}

#define copy_to_attribute(field)                                            \
    do {                                                                    \
        a->field##_size = field##_size;                                     \
        a->field = malloc(field##_size + 1);                                \
        memcpy(a->field, field, field##_size);                              \
        ((uint8_t*)a->field)[field##_size] = 0;                             \
    } while (0)

void gas_set_attribute (chunk* c,
                              size_t key_size, const void *key,
                              size_t value_size, const void *value)
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


void gas_set_attribute_string_pair(chunk* c, const char *key, const char *value)
{
    gas_set_attribute(c, strlen(key), key, strlen(value), value);
}


void gas_set_payload (chunk* c, size_t payload_size, const void *payload)
{
    copy_to_field(payload);
}

char* gas_get_payload_as_string (chunk* c)
{
    assert(((char*)c->payload)[c->payload_size] == '\0');
    return c->payload;
}

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

size_t gas_nb_children (chunk *c)
{
    return c->nb_children;
}

/*}}}*/
/* management {{{*/
/**
 * @todo finish and test
 */
void gas_update (chunk* c)
{
    int i;

    size_t sum;
    size_t a, b;

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
    fflush(stdout);
}
/*}}}*/
/* io {{{*/

#include <math.h>

void gas_write_encoded_num (int fd, size_t value)
{
    /*printf("%ld\n", value); */
    /*printf("0x%lx\n", value); */

    size_t i, coded_length;
    uint8_t byte, mask;
    size_t zero_count, zero_bytes, zero_bits;
    ssize_t si;  // a signed i

    for (i = 1; 1; i++) {
        /*if (value < (unsigned int)pow(2, 7*i)-2) {*/
        if (value < ((1L << (7L*i))-1L)) {
            break;
        }
        if ((i * 7L) > (sizeof(size_t) * 8L)) {
            // warning, close to overflow
            //i--;
            break;
        }
    }
    coded_length = i;  /* not including header */
    //printf("coded_length: %ld\n", i);

    zero_count = coded_length - 1;
    zero_bytes = zero_count / 8;
    zero_bits = zero_count % 8;

    //printf("zero_count: %ld\n", zero_count);
    //printf("zero_bytes: %ld\n", zero_bytes);
    //printf("zero_bits: %ld\n", zero_bits);
    //fflush(stdout);

    byte = 0x0;
    for (i = 0; i < zero_bytes; i++) {
        write(fd, &byte, 1);
    }

    mask = 0x80;
    mask >>= zero_bits;
    /*printf("mask: 0x%x\n", mask); */

    /* write the first masked byte */
    if ((coded_length - 1) <= sizeof(size_t)) {
//        printf("0x%lx\n", mask);
//        printf("value            0x%lx\n", value);
//        printf("hard shift value 0x%lx\n", value >> 8);
//        printf("coded length: %d\n", coded_length);
//        printf("shift amount: %ld\n", (coded_length-1L)*8L);
//        printf("shifted 0x%lx\n", ((value >> ((coded_length-1L)*8L))));
        byte = mask | ((value >> ((coded_length-zero_bytes-1)*8)) & 0xff);
//        printf("byte: 0x%lx\n", byte);
    } else {
        byte = mask;
    }
    /*printf("first: 0x%x\n", byte); */
    write(fd, &byte, 1);

    /*
     * write remaining bytes
     * from coded length, subtract 1 byte because we count down to zero
     * subtract an addition byte because one was already or'ed with the mask
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si*8)) & 0xff);
        /*printf("next byte: 0x%x\n", byte); */
        write(fd, &byte, 1);
    }

    fflush(stdout);
}

size_t gas_read_encoded_num (int fd)
{
    size_t retval;
    int i, bytes_read, zero_byte_count, first_bit_set;
    uint8_t byte, mask = 0x00;
    size_t additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        if (byte != 0x00)
            break;
    }

    /* process initial byte */
    for (first_bit_set = 7; first_bit_set >= 0; first_bit_set--)
        if (byte & (1L << first_bit_set))
            break;
    //assert(first_bit_set > 0);

    for (i = 0; i < first_bit_set; i++)
        mask |= (1L << i);

    additional_bytes_to_read = (7-first_bit_set) + (7*zero_byte_count);

    /* at this point, i have enough information to construct retval */
    retval = mask & byte;
    for (i = 0; i < additional_bytes_to_read; i++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        retval = (retval << 8) | byte;
    }
    return retval;
}

#define write_field(field)                                                  \
    do {                                                                    \
        gas_write_encoded_num(fd, self->field##_size);                      \
        write(fd, self->field, self->field##_size);                         \
    } while(0)

void gas_write (chunk* self, int fd)
{
    int i;

    /* this chunk's size */
    gas_write_encoded_num(fd, self->size);
    write_field(id);
    /* attributes */
    gas_write_encoded_num(fd, self->nb_attributes);
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    gas_write_encoded_num(fd, self->nb_children);
    for (i = 0; i < self->nb_children; i++) {
        gas_write(self->children[i], fd);
    }
}

#define read_field(field)                                                     \
    do {                                                                      \
        field##_size = gas_read_encoded_num(fd);                              \
        field = malloc(field##_size + 1);                                     \
        read(fd, field, field##_size);                                        \
        ((uint8_t*)field)[field##_size] = 0;                                  \
    } while (0)

chunk* gas_read (int fd)
{
    int i;
    chunk* c = gas_new(0, NULL);

    c->size = gas_read_encoded_num(fd);
    read_field(c->id);
    c->nb_attributes = gas_read_encoded_num(fd);
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    c->nb_children = gas_read_encoded_num(fd);
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        c->children[i] = gas_read(fd);
    }
    return c;
}

int gas_cmp (size_t a_len, const uint8_t *a, size_t b_len, const uint8_t *b)
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

/**
 * @return signed index
 * @retval -1 failure, attribute not found
 */
ssize_t gas_index_of_attribute (chunk* c, size_t key_size, const void* key)
{
    ssize_t i;
    attribute* a;
    for (i = 0; i < c->nb_attributes; i++ ) {
        a = &c->attributes[i];
        if (gas_cmp(a->key_size, a->key, key_size, key) == 0) {
            return i;
        }
    }
    return -1;
}

int gas_has_attribute (chunk* c, size_t key_size, void* key)
{
    return gas_index_of_attribute(c, key_size, key) == -1 ? 0 : 1;
}

chunk* gas_get_child_at (chunk* c, size_t index)
{
    if (index >= c->nb_children) {
        return NULL;
    }
    return c->children[index];
}

/**
 * @note This method does not allocate or copy value data.
 * @return request status
 * @retval 0 failure
 * @retval 1 success
 */
int gas_get_attribute (chunk* c,
                        size_t key_size, const void* key,
                        size_t* value_size, void** value)
{
    attribute* a;
    ssize_t index = gas_index_of_attribute(c, key_size, key);

    if (index == -1) {
        *value_size = 0;
        *value = NULL;
        return 0;
    }

    a = &c->attributes[index];
    *value_size = a->value_size;
    *value = a->value;
    return 1;
}

char* gas_get_attribute_string_pair (chunk* c, const char* key)
{
    int status;
    size_t value_size;
    char *value;

    status = gas_get_attribute(c, strlen(key), key, &value_size, (void**)&value);

    if (status == 0) {
        return NULL;
    }
    /* should already be null terminated */
    assert(value[value_size] == '\0');

    return value;
}

/*}}}*/

/* temporary string based debugging {{{ */

int level = 1;
int level_iter;
#define indent() for (level_iter=0;level_iter<level;level_iter++) printf("  ")

void gas_print (chunk* c)
{
    int i;

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

/* vim: set sw=4 fdm=marker : */
