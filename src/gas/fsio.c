
/**
 * @file fsio.c
 *
 * @brief File descriptor based io.
 */

#include "gas.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#if UNIX
#include <unistd.h>
#endif

/* gas_write_encoded_num_fs() {{{*/
void gas_write_encoded_num_fs (FILE* fs, GASunum value)
{
    GASunum i, coded_length;
    GASubyte byte, mask;
    GASunum zero_count, zero_bytes, zero_bits;
    GASnum si;  /* a signed i */

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
        fwrite(&byte, 1, 1, fs);
    }

    mask = 0x80;
    mask >>= zero_bits;

    /* write the first masked byte */
    if ((coded_length - 1) <= sizeof(GASunum)) {
        byte = mask | ((value >> ((coded_length-zero_bytes-1)*8)) & 0xff);
    } else {
        byte = mask;
    }
    fwrite(&byte, 1, 1, fs);

    /*
     * write remaining bytes
     * from coded length, subtract 1 byte because we count down to zero
     * subtract an addition byte because one was already or'ed with the mask
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si*8)) & 0xff);
        fwrite(&byte, 1, 1, fs);
    }
}
/*}}}*/
/* gas_write_fs() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        gas_write_encoded_num_fs(fs, self->field##_size);                   \
        fwrite(self->field, 1, self->field##_size, fs);                     \
    } while(0)

void gas_write_fs (chunk* self, FILE* fs)
{
    int i;

    /* this chunk's size */
    gas_write_encoded_num_fs(fs, self->size);
    write_field(id);
    /* attributes */
    gas_write_encoded_num_fs(fs, self->nb_attributes);
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    gas_write_encoded_num_fs(fs, self->nb_children);
    for (i = 0; i < self->nb_children; i++) {
        gas_write_fs(self->children[i], fs);
    }
}
/*}}}*/

/* gas_read_encoded_num_fs() {{{*/
GASunum gas_read_encoded_num_fs (FILE* fs)
{
    GASunum retval;
    int i, bytes_read, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = fread(&byte, 1, 1, fs);
        if (bytes_read == 0) {
            fprintf(stderr, "eof\n");
            abort();
        }
        if (bytes_read != 1) {
            fprintf(stderr, "error: %d: %s\n", errno, strerror(errno));
            abort();
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
        bytes_read = fread(&byte, 1, 1, fs);
        if (bytes_read == 0) {
            fprintf(stderr, "eof\n");
            abort();
        }
        if (bytes_read != 1) {
            fprintf(stderr, "error: %d: %s\n", errno, strerror(errno));
            abort();
        }
        retval = (retval << 8) | byte;
    }
    return retval;
}
/*}}}*/
/* gas_read_fs() {{{*/

#define read_field(field)                                                     \
    do {                                                                      \
        field##_size = gas_read_encoded_num_fs(fs);                           \
        field = malloc(field##_size + 1);                                     \
        fread(field, 1, field##_size, fs);                                        \
        ((GASubyte*)field)[field##_size] = 0;                                 \
    } while (0)

chunk* gas_read_fs (FILE* fs)
{
    int i;
    chunk* c = gas_new(0, NULL);
    c->size = gas_read_encoded_num_fs(fs);
    read_field(c->id);
    c->nb_attributes = gas_read_encoded_num_fs(fs);
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    c->nb_children = gas_read_encoded_num_fs(fs);
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        c->children[i] = gas_read_fs(fs);
        c->children[i]->parent = c;
    }
    return c;
}
/*}}}*/

/* vim: set sw=4 fdm=marker: */
