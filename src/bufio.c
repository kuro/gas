
#include "gas.h"
#include <string.h>

/* gas_buf_write_encoded_num() {{{*/
long gas_buf_write_encoded_num (GASubyte* buf, GASunum value)
{
    GASunum off = 0;
    GASunum i, coded_length;
    GASubyte byte, mask;
    GASunum zero_count, zero_bytes, zero_bits;
    long si;  /* a signed i */

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
    if ((coded_length - 1) <= sizeof(GASunum)) {
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
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si*8)) & 0xff);
        memcpy(buf+off, &byte, 1);
        off++;
    }

    return off;
}
/*}}}*/
/* gas_buf_write() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        off += gas_buf_write_encoded_num(buf+off, self->field##_size);      \
        memcpy(buf+off, self->field, self->field##_size);                  \
        off += self->field##_size; \
    } while(0)
long gas_buf_write (chunk* self, GASubyte* buf)
{
    int i;
    GASunum off = 0;

    /* this chunk's size */
    off += gas_buf_write_encoded_num(buf+off, self->size);
    write_field(id);
    /* attributes */
    off += gas_buf_write_encoded_num(buf+off, self->nb_attributes);
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    off += gas_buf_write_encoded_num(buf+off, self->nb_children);
    for (i = 0; i < self->nb_children; i++) {
        gas_buf_write(self->children[i], buf);
    }

    return off;
}
/*}}}*/

/* vim: set sw=4 fdm=marker : */
