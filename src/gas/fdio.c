
/**
 * @file fdio.c
 *
 * @brief File descriptor based io.
 */

#include <gas/gas.h>
#include <gas/fdio.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#if defined(UNIX) || defined(MINGW)
#include <unistd.h>
#endif

/* gas_write_encoded_num_fd() {{{*/
void gas_write_encoded_num_fd (int fd, GASunum value)
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
        write(fd, &byte, 1);
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
    write(fd, &byte, 1);

    /*
     * write remaining bytes
     * from coded length, subtract 1 byte because we count down to zero
     * subtract an addition byte because one was already or'ed with the mask
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si*8)) & 0xff);
        write(fd, &byte, 1);
    }
}
/*}}}*/
/* gas_read_encoded_num_fd() {{{*/
GASunum gas_read_encoded_num_fd (int fd)
{
    GASunum retval;
    int i, bytes_read, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read == 0) {
            return 0;
        }
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
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
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        retval = (retval << 8) | byte;
    }
    return retval;
}
/*}}}*/

/* gas_write_fd() {{{*/
#define write_field(field)                                                  \
    do {                                                                    \
        gas_write_encoded_num_fd(fd, self->field##_size);                   \
        write(fd, self->field, self->field##_size);                        \
    } while(0)

void gas_write_fd (chunk* self, int fd)
{
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
        gas_write_fd(self->children[i], fd);
    }
}
/*}}}*/
/* gas_read_fd() {{{*/

#define read_field(field)                                                     \
    do {                                                                      \
        field##_size = gas_read_encoded_num_fd(fd);                           \
        field = malloc(field##_size + 1);                                     \
        read(fd, field, field##_size);                                        \
        ((GASubyte*)field)[field##_size] = 0;                                 \
    } while (0)

chunk* gas_read_fd (int fd)
{
    int i;
    chunk* c = gas_new(NULL, 0);

    c->size = gas_read_encoded_num_fd(fd);

    /**
     * @todo gas_read_encoded_num_fd() returns an unsigned value.  This is a
     * bit hackish, but upon an end of file, it returns 0.  A chunk size is
     * never zero, so a result of 0 indicates eof.  Thus, clean up and return.
     */
    if (c->size == 0) {
        gas_destroy(c);
        return NULL;
    }

    read_field(c->id);
    c->nb_attributes = gas_read_encoded_num_fd(fd);
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
    }
    read_field(c->payload);
    c->nb_children = gas_read_encoded_num_fd(fd);
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        c->children[i] = gas_read_fd(fd);
        c->children[i]->parent = c;
    }
    return c;
}
/*}}}*/

/* vim: set sw=4 fdm=marker: */
