
/**
 * @file bufio.c
 * @brief Memory buffer based routines.
 *
 * @todo This should perform acceptable when there are no error conditions.
 * However, the sanity checks require review.
 *
 * @note The following functions are offset based.  As such, GAS_OK is not
 * returned upon success.  Instead, success is indicated by positive return
 * values.  Return values of 0 are not possible, as even an empty chunk
 * consumes 5 bytes.  Most importantly, negative return values are error
 * conditions.
 */

#include <gas/bufio.h>

#include <string.h>
#include <stdio.h>

/* gas_write_encoded_num_buf() {{{*/
/**
 * @return When positive, the new buffer offset.  Otherwise, an error code.
 *
 * @todo limit failure conditions
 */
GASnum gas_write_encoded_num_buf (GASubyte* buf, GASunum limit, GASunum value)
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
        if (off >= limit) {
            return GAS_ERR_UNKNOWN;
        }
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

    if (off >= limit) {
        return GAS_ERR_UNKNOWN;
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

        if (off >= limit) {
            return GAS_ERR_UNKNOWN;
        }

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
 *
 * @return When positive, the new buffer offset.  Otherwise, an error code.
 */
GASnum gas_read_encoded_num_buf (GASubyte* buf, GASunum limit, GASunum* result)
{
    GASunum offset;
    int i, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    offset = 0;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        byte = buf[offset++];
        if (offset > limit) {
            return GAS_ERR_UNKNOWN;
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

    /* at this point, i have enough information to construct *result */
    *result = mask & byte;
    for (i = 0; i < additional_bytes_to_read; i++) {
        byte = buf[offset++];
        if (offset > limit) {
            return GAS_ERR_UNKNOWN;
        }
        *result = (*result << 8) | byte;
    }

    return offset;
}
/*}}}*/

/* gas_write_buf() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        result = gas_write_encoded_num_buf(buf+off, 0, self->field##_size); \
        if (result <= 0) { return 0; }                                      \
        off += result;                                                      \
        memcpy(buf+off, self->field, self->field##_size);                   \
        off += self->field##_size;                                          \
    } while(0)

/**
 * @return When positive, the new buffer offset.  Otherwise, an error code.
 *
 * @todo limit failure conditions
 * @todo changes zeros to limits
 */
GASnum gas_write_buf (GASubyte* buf, GASunum limit, chunk* self)
{
    GASresult result;
    GASunum i;
    GASnum off = 0;

    /* this chunk's size */
    off += gas_write_encoded_num_buf(buf+off, 0, self->size);
    write_field(id);
    /* attributes */
    off += gas_write_encoded_num_buf(buf+off, 0, self->nb_attributes);
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    off += gas_write_encoded_num_buf(buf+off, 0, self->nb_children);
    for (i = 0; i < self->nb_children; i++) {
        /// @todo replace the 0
        result = gas_write_buf(buf + off, 0, self->children[i]);
        if (result <= 0) { return result; }
        off += result;
    }

    return off;
}
/*}}}*/
/* gas_read_buf() {{{*/

#define read_field(field)                                                   \
    do {                                                                    \
        read_num(field##_size);                                             \
        field = malloc(field##_size + 1);                                   \
        memcpy(field, buf+offset, field##_size);                            \
        offset += field##_size;                                             \
        ((GASubyte*)field)[field##_size] = 0;                               \
    } while (0)

/**
 * @todo redundant error checking
 */
#define read_num(field) \
    result = gas_read_encoded_num_buf(buf + offset, limit - offset, &field); \
    if (result <= 1) {                                                      \
        gas_destroy(c);                                                     \
        return result;                                                      \
    }                                                                       \
    offset += result;

GASnum gas_read_buf (GASubyte* buf, GASunum limit, chunk** out)
{
    GASresult result;
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
        result = gas_read_buf(buf + offset, limit - offset, &c->children[i]);
        if (result <= 0) {
            gas_destroy(c);
            return result;
        }
        c->children[i]->parent = c;
        offset += result;
    }

    *out = c;
    return offset;
}
/*}}}*/

/* vim: set sw=4 fdm=marker : */
