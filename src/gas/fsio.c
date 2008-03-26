
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
void gas_write_encoded_num_fs (FILE* fs, GASunum value)
{
    GASunum i, coded_length;
    GASubyte byte, mask;
    GASunum zero_count, zero_bytes, zero_bits;
    GASnum si;  /* a signed i */

/*    printf("orig %lx\n", value);*/

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
/*    printf("coded length %ld\n", coded_length);*/

    zero_count = coded_length - 1;
    zero_bytes = zero_count / 8;
    zero_bits = zero_count % 8;

/*    printf("zero count %ld\n", zero_count);*/
/*    printf("zero bytes %ld\n", zero_bytes);*/
/*    printf("zero bits %ld\n", zero_bits);*/

    byte = 0x0;
    for (i = 0; i < zero_bytes; i++) {
/*        printf("writing %x\n", byte);*/
        if (fwrite(&byte, 1, 1, fs) != 1) {
            gas_error = GAS_ERR_UNKNOWN;
            goto abort;
        }
    }

    mask = 0x80;
    mask >>= zero_bits;
/*    printf("mask %x\n", mask);*/

    /* write the first masked byte */
    /*if ((coded_length - 1) <= sizeof(GASunum)) {*/
    if (coded_length <= sizeof(GASunum)) {
        byte = mask | ((value >> ((coded_length-zero_bytes-1)*8)) & 0xff);
    } else {
        byte = mask;
    }
/*    printf("writing %x\n", byte);*/
    if (fwrite(&byte, 1, 1, fs) != 1) {
        gas_error = GAS_ERR_UNKNOWN;
        goto abort;
    }

    /*
     * write remaining bytes
     * from coded length, subtract 1 byte because we count down to zero
     * subtract an addition byte because one was already or'ed with the mask
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si*8)) & 0xff);
/*        printf("writing %x\n", byte);*/
        if (fwrite(&byte, 1, 1, fs) != 1) {
            gas_error = GAS_ERR_UNKNOWN;
            goto abort;
        }
    }

abort:
    return;
}
/*}}}*/
/* gas_write_fs() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        gas_write_encoded_num_fs(fs, self->field##_size);                   \
        if (gas_error != GAS_OK) {                                          \
            goto abort;                                                     \
        }                                                                   \
        if (fwrite(self->field,1,self->field##_size,fs)!=self->field##_size)\
        {                                                                   \
            goto abort;                                                     \
        }                                                                   \
    } while(0)

void gas_write_fs (chunk* self, FILE* fs)
{
    int i;

    /* this chunk's size */
    gas_write_encoded_num_fs(fs, self->size);
    if (gas_error != GAS_OK) {
        goto abort;
    }
    write_field(id);
    /* attributes */
    gas_write_encoded_num_fs(fs, self->nb_attributes);
    if (gas_error != GAS_OK) {
        goto abort;
    }
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    gas_write_encoded_num_fs(fs, self->nb_children);
    if (gas_error != GAS_OK) {
        goto abort;
    }
    for (i = 0; i < self->nb_children; i++) {
        gas_write_fs(self->children[i], fs);
        if (gas_error != GAS_OK) {
            goto abort;
        }
    }

abort:
    return;
}
/*}}}*/

/* gas_read_encoded_num_fs() {{{*/
GASunum gas_read_encoded_num_fs (FILE* fs)
{
    GASunum retval = 0x0;
    int i, bytes_read, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = fread(&byte, 1, 1, fs);
        if (bytes_read == 0) {
            gas_error = GAS_ERR_UNKNOWN;
            return 0;
        }
        if (bytes_read != 1) {
            gas_error = GAS_ERR_UNKNOWN;
            return 0;
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
            gas_error = GAS_ERR_UNKNOWN;
            return 0;
        }
        if (bytes_read != 1) {
            gas_error = GAS_ERR_UNKNOWN;
            return 0;
        }
        retval = (retval << 8) | byte;
    }
    return retval;
}
/*}}}*/
/* gas_read_fs() {{{*/

#define read_field(field)                                                   \
    do {                                                                    \
        field##_size = gas_read_encoded_num_fs(fs);                         \
        if (gas_error != GAS_OK) { goto abort; }                            \
        field = malloc(field##_size + 1);                                   \
        if (fread(field, 1, field##_size, fs) != field##_size) {            \
            gas_error = GAS_ERR_UNKNOWN;                                    \
            goto abort;                                                     \
        }                                                                   \
        ((GASubyte*)field)[field##_size] = 0;                               \
    } while (0)

chunk* gas_read_fs (FILE* fs)
{
    int i;
    chunk* c = gas_new(NULL, 0);
    c->size = gas_read_encoded_num_fs(fs);
    if (gas_error != GAS_OK) { goto abort; }
    read_field(c->id);
    c->nb_attributes = gas_read_encoded_num_fs(fs);
    if (gas_error != GAS_OK) { goto abort; }
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    c->nb_children = gas_read_encoded_num_fs(fs);
    if (gas_error != GAS_OK) { goto abort; }
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        c->children[i] = gas_read_fs(fs);
        if (gas_error != GAS_OK) { goto abort; }
        c->children[i]->parent = c;
    }
    return c;

abort:
    return NULL;
}
/*}}}*/

/* vim: set sw=4 fdm=marker: */
