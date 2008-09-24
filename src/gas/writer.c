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

#include <gas/writer.h>

#include <string.h>


/* gas_write_encoded_num_writer() {{{*/
GASresult gas_write_encoded_num_writer (GASwriter *writer, GASunum value)
{
    GASresult result = GAS_OK;
    unsigned int bytes_written;
    GASunum i, coded_length;
    GASubyte byte, mask;
    GASunum zero_count, zero_bytes, zero_bits;
    GASnum si;  /* a signed i */

    GAS_CHECK_PARAM(writer);

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
        byte = mask | ((value >> ((coded_length-zero_bytes-1)<<3)) & 0xff);
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
     *
     * @internal
     * @todo figure out why zero_bytes is subtracted
     */
    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si<<3)) & 0xff);
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

GASresult gas_write_writer (GASwriter *writer, GASchunk* self)
{
    GASresult result = GAS_OK;
    GASunum i;
    unsigned int bytes_written;

    GAS_CHECK_PARAM(writer);
    GAS_CHECK_PARAM(self);

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
    if (self->payload) {
        write_field(payload);
    } else {
        if (writer->on_write_payload == NULL) {
            return GAS_ERR_INVALID_PAYLOAD;
        }
        result = writer->on_write_payload(writer, self, &bytes_written);
        if (result != GAS_OK) { return result; }
    }
    /* children */
    result = gas_write_encoded_num_writer(writer, self->nb_children);
    if (result != GAS_OK) { return result; }
    for (i = 0; i < self->nb_children; i++) {
        gas_write_writer(writer, self->children[i]);
    }

    return result;
}
/*}}}*/

GASwriter* gas_writer_new (GAScontext* context)/*{{{*/
{
    GASwriter *w;

#ifdef GAS_DEBUG
    if (context == NULL) { return NULL; }
#endif

    w = (GASwriter*)malloc(sizeof(GASwriter));
    if (w == NULL) { return NULL; }

    memset(w, 0, sizeof(GASwriter));

    w->context = context;

    return w;
}/*}}}*/

void gas_writer_destroy (GASwriter *w)/*{{{*/
{
    free(w);
}/*}}}*/

GASresult gas_write (GASwriter* w, const char *resource, GASchunk *c)
{
    GASresult result;
    GAScontext *ctx = NULL;

    GAS_CHECK_PARAM(w);
    GAS_CHECK_PARAM(resource);
    GAS_CHECK_PARAM(c);

    ctx = w->context;

    result = ctx->open(resource, "wb", &w->handle, &ctx->user_data);
    if (result < GAS_OK) {
        return result;
    }
    result = gas_write_writer(w, c);
    if (result < GAS_OK) {
        return result;
    }
    result = ctx->close(w->handle, ctx->user_data);
    return result;
}

/* vim: set sw=4 fdm=marker :*/
