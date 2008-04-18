
#include <gas/writer.h>


/* gas_write_encoded_num_writer() {{{*/
GASresult gas_write_encoded_num_writer (gas_writer *writer, GASunum value)
{
    GASresult result = GAS_OK;
    unsigned int bytes_written;
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
        result = writer->context->write(writer->handle, &byte, 1, &bytes_written, writer->context->user_data);
        if (result != 0) {
            return result;
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
    result = writer->context->write(writer->handle, &byte, 1, &bytes_written, writer->context->user_data);
    if (result != 0) {
        return result;
    }

    /*
     * write remaining bytes
     * from coded length, subtract 1 byte because we count down to zero
     * subtract an addition byte because one was already or'ed with the mask
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si*8)) & 0xff);
        result = writer->context->write(writer->handle, &byte, 1, &bytes_written, writer->context->user_data);
        if (result != 0) {
            return result;
        }
    }

    return GAS_OK;
}
/*}}}*/


/* gas_write_writer() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        result = gas_write_encoded_num_writer(writer, self->field##_size);  \
        if (result != GAS_OK) { return result; }                            \
        result = writer->context->write(                                    \
            writer->handle, self->field,                                    \
            self->field##_size, &bytes_written,                             \
            writer->context->user_data);                                    \
        if (result != GAS_OK) { return result; }                            \
    } while(0)

GASresult gas_write_writer (gas_writer *writer, chunk* self)
{
    GASresult result = GAS_OK;
    int i;
    unsigned int bytes_written;

    /* this chunk's size */
    gas_write_encoded_num_writer(writer, self->size);
    if (result != GAS_OK) { return result; }
    write_field(id);
    /* attributes */
    gas_write_encoded_num_writer(writer, self->nb_attributes);
    if (result != GAS_OK) { return result; }
    for (i = 0; i < self->nb_attributes; i++) {
        write_field(attributes[i].key);
        write_field(attributes[i].value);
    }
    write_field(payload);
    /* children */
    result = gas_write_encoded_num_writer(writer, self->nb_children);
    if (result != GAS_OK) { return result; }
    for (i = 0; i < self->nb_children; i++) {
        gas_write_writer(writer, self->children[i]);
    }

    return result;
}
/*}}}*/

/* vim: set sw=4 fdm=marker :*/
