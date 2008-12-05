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
 * @todo reconsider read() write() return values
 */

#include <gas/fdio.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef MSVC
GASnum read (int fd, GASvoid *buf, GASunum count);
GASnum write (int fd, const GASvoid *buf, GASunum count);
#endif

/* gas_write_encoded_num_fd() {{{*/
GASresult gas_write_encoded_num_fd (int fd, GASunum value)
{
    GASunum i, coded_length;
    GASubyte byte, mask;
    GASunum zero_count, zero_bytes, zero_bits;
    GASnum si;  /* a signed i */
    GASnum bytes_written;

    for (i = 1; 1; i++) {
        if (value < ((1UL << (7UL*i))-1UL)) {
            break;
        }
        if ((i * 7L) > (sizeof(GASunum) << 3)) {
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
        byte = mask | ((value >> ((coded_length-zero_bytes-1)<<3)) & 0xff);
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
     *
     * @internal
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si<<3)) & 0xff);
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
    GASunum i, bytes_read, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    GAS_CHECK_PARAM(value);

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
        if (result != GAS_OK) { return result; }                            \
        write(fd, self->field, self->field##_size);                         \
    } while(0)

GASresult gas_write_fd (int fd, GASchunk* self)
{
    GASresult result;
    GASunum i;

    GAS_CHECK_PARAM(self);

    /* this GASchunk's size */
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
        field = (GASubyte*)gas_alloc(field##_size + 1);                     \
        GAS_CHECK_MEM(field);                                               \
        bytes_read = read(fd, field, field##_size);                         \
        if (bytes_read < 0) { return GAS_ERR_UNKNOWN; }                     \
        if (bytes_read != (GASnum)field##_size) {return GAS_ERR_UNKNOWN;}   \
        ((GASubyte*)field)[field##_size] = 0;                               \
    } while (0)

GASresult gas_read_fd (int fd, GASchunk** out)
{
    GASresult result;
    GASunum i;
    GASchunk* c = gas_new(NULL, 0);
    GASnum bytes_read;

    result = gas_read_encoded_num_fd(fd, &c->size);
    if (result != GAS_OK) {
        return result;
    }

    if (c->size == 0) {
        gas_destroy(c);
        return GAS_ERR_UNKNOWN;
    }

    read_field(c->id);
    result = gas_read_encoded_num_fd(fd, &c->nb_attributes);
    if (result != GAS_OK) {
        return result;
    }
    c->attributes = (GASattribute*)gas_alloc(c->nb_attributes * sizeof(GASattribute));
    GAS_CHECK_MEM(c->attributes);
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    result = gas_read_encoded_num_fd(fd, &c->nb_children);
    if (result != GAS_OK) {
        return result;
    }
    c->children = (GASchunk**)gas_alloc(c->nb_children * sizeof(GASchunk*));
    GAS_CHECK_MEM(c->children);
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
