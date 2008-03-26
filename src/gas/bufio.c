
/**
 * @file bufio.c
 * @brief Memory buffer based routines.
 *
 * @todo This should perform acceptable when there are no error conditions.
 * However, the sanity checks require review.
 */

#include <gas/bufio.h>
#include <gas/ntstring.h>

#include <string.h>
#include <stdio.h>

/* gas_write_encoded_num_buf() {{{*/
GASnum gas_write_encoded_num_buf (GASubyte* buf, GASunum value)
{
    GASnum off = 0;
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
            /*i--;*/
            break;
        }
    }
    coded_length = i;  /* not including header */

    zero_count = coded_length - 1;
    zero_bytes = zero_count / 8;
    zero_bits = zero_count % 8;

    byte = 0x0;
    for (i = 0; i < zero_bytes; i++) {
        memcpy(buf+off, &byte, 1);
        off++;
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
    memcpy(buf+off, &byte, 1);
    off++;

    /*
     * write remaining bytes
     * from coded length, subtract 1 byte because we count down to zero
     * subtract an addition byte because one was already or'ed with the mask
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = (GASnum)(coded_length - 2 - zero_bytes); si >= 0; si--) {
        byte = (GASubyte)((value >> ((GASunum)si*8)) & 0xffL);
        memcpy(buf+off, &byte, 1);
        off++;
    }

    return off;
}
/*}}}*/
/* gas_read_encoded_num_buf() {{{*/
/**
 * @brief decode a number from a memory buffer.
 * @param limit The length of the buffer.  Serves as a limiter.  However, limit
 * bytes may or may not be used.
 * @return The status.  When >0, the number of bytes used.  When <=0, error.
 * @retval 0 error
 */
GASnum gas_read_encoded_num_buf (GASubyte* buf, GASunum limit, GASunum* result)
{
    GASunum offset;
    GASunum retval;
    int i, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    offset = 0;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        byte = buf[offset++];
        if (offset > limit) {
            puts("offset was limit");
            gas_error = GAS_ERR_UNKNOWN;
            return GAS_FALSE;
        }
        if (byte != 0x00)
            break;
    }

    /* process initial byte */
    for (first_bit_set = 7; first_bit_set >= 0; first_bit_set--)
        if (byte & (1L << first_bit_set))
            break;
/*    assert(first_bit_set > 0);*/

    for (i = 0; i < first_bit_set; i++)
        mask |= (1L << i);

    additional_bytes_to_read = (7-first_bit_set) + (7*zero_byte_count);

    /* at this point, i have enough information to construct retval */
    retval = mask & byte;
    for (i = 0; i < additional_bytes_to_read; i++) {
        byte = buf[offset++];
        if (offset > limit) {
            puts("offset was limit 2");
            gas_error = GAS_ERR_UNKNOWN;
            return GAS_FALSE;
        }
        retval = (retval << 8) | byte;
    }
    *result = retval;
    return offset;
}
/*}}}*/

/* gas_write_buf() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        off += gas_write_encoded_num_buf(buf+off, self->field##_size);      \
        if (gas_error != GAS_OK) { return 0; }                              \
        memcpy(buf+off, self->field, self->field##_size);                   \
        off += self->field##_size;                                          \
    } while(0)
GASnum gas_write_buf (chunk* self, GASubyte* buf)
{
    GASunum i;
    GASnum off = 0;

    /* this chunk's size */
    off += gas_write_encoded_num_buf(buf+off, self->size);
    write_field(id);
    /* attributes */
    off += gas_write_encoded_num_buf(buf+off, self->nb_attributes);
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    off += gas_write_encoded_num_buf(buf+off, self->nb_children);
    for (i = 0; i < self->nb_children; i++) {
        off += gas_write_buf(self->children[i], buf + off);
        if (gas_error != GAS_OK) { return 0; }
    }

    return off;
}
/*}}}*/
/* gas_read_buf() {{{*/

#define read_field(field)                                                     \
    do {                                                                      \
        read_num(field##_size);                                               \
        field = malloc(field##_size + 1);                                     \
        memcpy(field, buf+offset, field##_size);                              \
        offset += field##_size;                                               \
        ((GASubyte*)field)[field##_size] = 0;                                 \
    } while (0)

/**
 * @todo redundant error checking
 */
#define read_num(field) \
    tmp = gas_read_encoded_num_buf(buf + offset, limit - offset, &field); \
    if (tmp < 1 || gas_error != GAS_OK) {                                 \
        gas_destroy(c);                                                   \
        return NULL;                                                      \
    }                                                                     \
    offset += tmp;


chunk* gas_read_buf (GASubyte* buf, GASunum limit, GASnum* out_offset)
{
    GASnum tmp;
    GASunum offset = 0;

    int i;
    chunk* c = gas_new(NULL, 0);

    read_num(c->size);
    read_field(c->id);
    read_num(c->nb_attributes);
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    read_num(c->nb_children);
    c->children = malloc(c->nb_children * sizeof(chunk*));
    memset(c->children, 0, c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        c->children[i] = gas_read_buf(buf + offset, limit - offset, &tmp);
        c->children[i]->parent = c;
        if (c->children[i] == NULL || gas_error != GAS_OK) {
            gas_destroy(c);
            return NULL;
        }
        offset += tmp;
    }

    if (out_offset != NULL) {
        *out_offset = offset;
    }

    return c;
}
/*}}}*/

/* vim: set sw=4 fdm=marker : */
