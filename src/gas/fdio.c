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
 * @file fdio.c
 *
 * @brief File descriptor based io.
 */

#include <gas/fdio.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

/* gas_write_encoded_num_fd() {{{*/
GASresult gas_write_encoded_num_fd (int fd, GASunum value)
{
    GASunum i, coded_length;
    GASubyte byte, mask;
    GASunum zero_count, zero_bytes, zero_bits;
    GASnum si;  /* a signed i */
    ssize_t bytes_written;

    for (i = 1; 1; i++) {
        if (value < ((1L << (7L*i))-1L)) {
            break;
        }
        if ((i * 7L) > (sizeof(GASunum) * 8L)) {
            /* warning, close to overflow */
            /* i--; */
            break;
        }
    }
    coded_length = i;  /* not including header */

    zero_count = coded_length - 1;
    zero_bytes = zero_count / 8;
    zero_bits = zero_count % 8;

    byte = 0x0;
    for (i = 0; i < zero_bytes; i++) {
        bytes_written = write(fd, &byte, 1);
        if (bytes_written != 1) {
            return GAS_ERR_UNKNOWN;
        }
    }

    mask = 0x80;
    mask >>= zero_bits;

    /* write the first masked byte */
    /*if ((coded_length - 1) <= sizeof(GASunum)) {*/
    if (coded_length <= sizeof(GASunum)) {
        byte = mask | ((value >> ((coded_length-zero_bytes-1)*8)) & 0xff);
    } else {
        byte = mask;
    }
    bytes_written = write(fd, &byte, 1);
    if (bytes_written != 1) {
        return GAS_ERR_UNKNOWN;
    }

    /*
     * write remaining bytes
     * from coded length, subtract 1 byte because we count down to zero
     * subtract an addition byte because one was already or'ed with the mask
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si*8)) & 0xff);
        bytes_written = write(fd, &byte, 1);
        if (bytes_written != 1) {
            return GAS_ERR_UNKNOWN;
        }
    }

    return GAS_OK;
}
/*}}}*/
/* gas_read_encoded_num_fd() {{{*/
GASresult gas_read_encoded_num_fd (int fd, GASunum* value)
{
    GASunum retval;
    int i, bytes_read, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            return GAS_ERR_UNKNOWN;
        }
        if (byte != 0x00)
            break;
    }

    /* process initial byte */
    for (first_bit_set = 7; first_bit_set >= 0; first_bit_set--)
        if (byte & (1L << first_bit_set))
            break;

    for (i = 0; i < first_bit_set; i++)
        mask |= (1L << i);

    additional_bytes_to_read = (7-first_bit_set) + (7*zero_byte_count);

    /* at this point, i have enough information to construct retval */
    retval = mask & byte;
    for (i = 0; i < additional_bytes_to_read; i++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            return GAS_ERR_UNKNOWN;
        }
        retval = (retval << 8) | byte;
    }

    *value = retval;
    return GAS_OK;
}
/*}}}*/

/* gas_write_fd() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        result = gas_write_encoded_num_fd(fd, self->field##_size);          \
        if (result != self->field##_size) { return result; }                \
        write(fd, self->field, self->field##_size);                         \
    } while(0)

GASresult gas_write_fd (int fd, chunk* self)
{
    GASresult result;
    int i;

    /* this chunk's size */
    gas_write_encoded_num_fd(fd, self->size);
    write_field(id);
    /* attributes */
    gas_write_encoded_num_fd(fd, self->nb_attributes);
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    gas_write_encoded_num_fd(fd, self->nb_children);
    for (i = 0; i < self->nb_children; i++) {
        result = gas_write_fd(fd, self->children[i]);
        if (result != GAS_OK) {
            return result;
        }
    }

    return GAS_OK;
}
/*}}}*/
/* gas_read_fd() {{{*/

#define read_field(field)                                                   \
    do {                                                                    \
        result = gas_read_encoded_num_fd(fd, &field##_size);                \
        if (result != GAS_OK) { return result; }                            \
        field = malloc(field##_size + 1);                                   \
        bytes_read = read(fd, field, field##_size);                         \
        if (bytes_read != field##_size) { return GAS_ERR_UNKNOWN; }         \
        ((GASubyte*)field)[field##_size] = 0;                               \
    } while (0)

GASresult gas_read_fd (int fd, chunk** out)
{
    GASresult result;
    int i;
    chunk* c = gas_new(NULL, 0);
    ssize_t bytes_read;

    result = gas_read_encoded_num_fd(fd, &c->size);
    if (result != GAS_OK) {
        return result;
    }

    /**
     * @todo gas_read_encoded_num_fd() returns an unsigned value.  This is a
     * bit hackish, but upon an end of file, it returns 0.  A chunk size is
     * never zero, so a result of 0 indicates eof.  Thus, clean up and return.
     */
    if (c->size == 0) {
        gas_destroy(c);
        return GAS_ERR_UNKNOWN;
    }

    read_field(c->id);
    result = gas_read_encoded_num_fd(fd, &c->nb_attributes);
    if (result != GAS_OK) {
        return result;
    }
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    result = gas_read_encoded_num_fd(fd, &c->nb_children);
    if (result != GAS_OK) {
        return result;
    }
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        result = gas_read_fd(fd, &c->children[i]);
        if (result != GAS_OK) {
            gas_destroy(c);
            return result;
        }
        c->children[i]->parent = c;
    }

    *out = c;
    return GAS_OK;
}
/*}}}*/

/* vim: set sw=4 fdm=marker: */
