
/**
 * @file fsio.c
 *
 * @brief File descriptor based io.
 */

#include <gas/fsio.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/* gas_write_encoded_num_fs() {{{*/
GASresult gas_write_encoded_num_fs (FILE* fs, GASunum value)
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
        if (fwrite(&byte, 1, 1, fs) != 1) {
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
    if (fwrite(&byte, 1, 1, fs) != 1) {
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
        if (fwrite(&byte, 1, 1, fs) != 1) {
            return GAS_ERR_UNKNOWN;
        }
    }

    return GAS_OK;
}
/*}}}*/
/* gas_write_fs() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        result = gas_write_encoded_num_fs(fs, self->field##_size);          \
        if (result != GAS_OK) {                                             \
            return result;                                                  \
        }                                                                   \
        if (fwrite(self->field,1,self->field##_size,fs)!=self->field##_size)\
        {                                                                   \
            return GAS_ERR_UNKNOWN;                                         \
        }                                                                   \
    } while(0)

GASresult gas_write_fs (FILE* fs, chunk* self)
{
    GASresult result;
    int i;

    /* this chunk's size */
    result = gas_write_encoded_num_fs(fs, self->size);
    if (result != GAS_OK) {
        return result;
    }
    write_field(id);
    /* attributes */
    result = gas_write_encoded_num_fs(fs, self->nb_attributes);
    if (result != GAS_OK) {
        return result;
    }
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    result = gas_write_encoded_num_fs(fs, self->nb_children);
    if (result != GAS_OK) {
        return result;
    }
    for (i = 0; i < self->nb_children; i++) {
        result = gas_write_fs(fs, self->children[i]);
        if (result != GAS_OK) {
            return result;
        }
    }

    return GAS_OK;
}
/*}}}*/

/* gas_read_encoded_num_fs() {{{*/
GASresult gas_read_encoded_num_fs (FILE* fs, GASunum *value)
{
    GASunum retval = 0x0;
    int i, bytes_read, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = fread(&byte, 1, 1, fs);
        if (bytes_read == 0) {
            return GAS_ERR_UNKNOWN;
        }
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
        bytes_read = fread(&byte, 1, 1, fs);
        if (bytes_read == 0) {
            return GAS_ERR_UNKNOWN;
        }
        if (bytes_read != 1) {
            return GAS_ERR_UNKNOWN;
        }
        retval = (retval << 8) | byte;
    }

    *value = retval;
    return GAS_OK;
}
/*}}}*/
/* gas_read_fs() {{{*/

#define read_field(field)                                                   \
    do {                                                                    \
        result = gas_read_encoded_num_fs(fs, &field##_size);                 \
        if (result != GAS_OK) { return result; }                            \
        field = malloc(field##_size + 1);                                   \
        if (fread(field, 1, field##_size, fs) != field##_size) {            \
            return GAS_ERR_UNKNOWN;                                         \
        }                                                                   \
        ((GASubyte*)field)[field##_size] = 0;                               \
    } while (0)

GASresult gas_read_fs (FILE* fs, chunk **out)
{
    GASresult result;
    int i;
    chunk* c = gas_new(NULL, 0);

    result = gas_read_encoded_num_fs(fs, &c->size);
    if (result != GAS_OK) { return result; }
    read_field(c->id);
    result = gas_read_encoded_num_fs(fs, &c->nb_attributes);
    if (result != GAS_OK) { return result; }
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    result = gas_read_encoded_num_fs(fs, &c->nb_children);
    if (result != GAS_OK) { return result; }
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
         result = gas_read_fs(fs, &c->children[i]);
        if (result != GAS_OK) { return result; }
        c->children[i]->parent = c;
    }

    *out = c;
    return GAS_OK;
}
/*}}}*/

/* vim: set sw=4 fdm=marker: */
