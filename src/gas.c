
/**
 * @file gas.c
 * @brief gas implementation
 */

#include "gas.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void gas_print (chunk* c);

inline
size_t encoded_size (size_t value)
{
#if FIXED
    return sizeof(uint32_t);
#else
    int i, coded_length;

    for (i = 1; 1; i++) {
        if (value < (1 << (7*i-1))) {
            break;
        }
    }
    coded_length = i;  // not including header
    //printf("coded_length: %d\n", coded_length);

    int zero_count = coded_length - 1;
    int zero_bytes = zero_count / 8;
    //int zero_bits = zero_count % 8;

    return coded_length + zero_bytes;
#endif
}

#define copy_to_field(field)                                                \
    do {                                                                    \
        c->field##_size = field##_size;                                     \
        c->field = malloc(field##_size + 1);                                \
        memcpy(c->field, field, field##_size);                              \
        ((uint8_t*)c->field)[field##_size] = 0;                             \
    } while (0)

/* cons/decons {{{*/
chunk* gas_new (size_t id_size, void *id)
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


chunk* gas_new_named (char *id)
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

void gas_set_id (chunk* c, size_t id_size, void *id)
{
    copy_to_field(id);
}


#define copy_to_attribute(field)                                            \
    do {                                                                    \
        a->field##_size = field##_size;                                     \
        a->field = malloc(field##_size + 1);                                \
        memcpy(a->field, field, field##_size);                              \
        ((uint8_t*)a->field)[field##_size] = 0;                             \
    } while (0)

void gas_set_attribute (chunk* c,
                              size_t key_size, void *key,
                              size_t value_size, void *value)
{
    c->nb_attributes++;

    attribute* tmp = realloc(c->attributes, c->nb_attributes*sizeof(attribute));
    assert(tmp);
    c->attributes = tmp;

    attribute *a = &c->attributes[c->nb_attributes-1];
    copy_to_attribute(key);
    copy_to_attribute(value);
}


void gas_set_attribute_string_pair(chunk* c,
                                         char *key,
                                         char *value)
{
    gas_set_attribute(c, strlen(key), key, strlen(value), value);
}


void gas_set_payload (chunk* c, size_t payload_size, void *payload)
{
    copy_to_field(payload);
}


void gas_add_child(chunk* parent, chunk* child)
{
    parent->nb_children++;

    chunk** tmp = realloc(parent->children, parent->nb_children*sizeof(chunk*));
    assert(tmp);
    parent->children = tmp;

    parent->children[parent->nb_children - 1] = child;
    child->parent = parent;
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

    sum = 0;
    // id
    sum += encoded_size(c->id_size);
    sum += c->id_size;
    // attributes
    sum += encoded_size(c->nb_attributes);
    for (i = 0; i < c->nb_attributes; i++) {
        sum += encoded_size(c->attributes[i].key_size);
        sum += c->attributes[i].key_size;
        sum += encoded_size(c->attributes[i].value_size);
        sum += c->attributes[i].value_size;
    }
    // payload
    sum += encoded_size(c->payload_size);
    sum += c->payload_size;
    // children
    sum += encoded_size(c->nb_children);
    for (i = 0; i < c->nb_children; i++) {
        chunk* child = c->children[i];
        gas_update(child);
        sum += child->size;
    }

    // @todo what about summing own size?
    // this is just a best guess, and i don't like it
    sum += encoded_size(sum + 2);

    //printf("size: %ld\n", sum);
    c->size = sum;
    fflush(stdout);
}
/*}}}*/
/* io {{{*/

#include <arpa/inet.h>
void gas_write_encoded_num (int fd, size_t value)
{
#if FIXED
    uint32_t tmp = htonl(value);
    write(fd, &tmp, sizeof(tmp));
#else
    //printf("%ld\n", value);
    //printf("0x%lx\n", value);

    int i, coded_length;
    uint8_t byte, mask;

    for (i = 1; 1; i++) {
        if (value < (1 << (7*i-1))) {
            break;
        }
    }
    coded_length = i;  // not including header
    //printf("coded_length: %d\n", coded_length);

    int zero_count = coded_length - 1;
    int zero_bytes = zero_count / 8;
    int zero_bits = zero_count % 8;

    byte = 0x0;
    for (i = 0; i < zero_bytes; i++) {
        write(fd, &byte, 1);
    }

    mask = 0x80;
    mask >>= zero_bits;
    //printf("mask: 0x%x\n", mask);

    // write the first masked byte
    byte = mask | ((value >> ((coded_length-1)*8)) & 0xff);
    //printf("first: 0x%x\n", byte);
    write(fd, &byte, 1);

    // write remaining bytes
    for (i = coded_length - 2; i >= 0; i--) {
        byte = ((value >> (i*8)) & 0xff);
        //printf("next byte: 0x%x\n", byte);
        write(fd, &byte, 1);
    }

    fflush(stdout);
#endif
}

size_t gas_read_encoded_num (int fd)
{
#if FIXED
    uint32_t retval;
    read(fd, &tmp, sizeof(tmp));
    tmp = ntohl(tmp);
#else
    size_t retval;
    int i, bytes_read, zero_byte_count, first_bit_set;
    uint8_t byte, mask = 0x00;

    // find first non 0x00 byte
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        if (byte != 0x00)
            break;
    }

    // process initial byte
    for (first_bit_set = 7; first_bit_set >= 0; first_bit_set--)
        if (byte & (1L << first_bit_set))
            break;
    assert(first_bit_set > 0);

    for (i = 0; i < first_bit_set; i++)
        mask |= (1L << i);

    size_t additional_bytes_to_read = (7-first_bit_set) + (7*zero_byte_count);

    // at this point, i have enough information to construct retval
    retval = mask & byte;
    for (i = 0; i < additional_bytes_to_read; i++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        retval = (retval << 8) | byte;
    }
#endif
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

    // this chunk's size
    gas_write_encoded_num(fd, self->size);
    write_field(id);
    // attributes
    gas_write_encoded_num(fd, self->nb_attributes);
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    // children
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
    printf("---\n%s\n---\n", (char*)c->payload);

    level++;
    for (i = 0; i < c->nb_children; i++) {
        gas_print(c->children[i]);
    }
    level--;
}

/* }}} */

// vim: sw=4 fdm=marker
