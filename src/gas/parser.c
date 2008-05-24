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

/* page: The Gas Parser {{{*/
/**
 * @page parser The Gas Parser
 *
 * Of all the gas input/output methods, the Gas Parser is the most powerful and
 * flexible.
 *
 * @section parser_tuning Parser Tuning
 *
 * The parser's behavior is determined based upon a few factors, including the
 * seetings GASparser::build_tree, as well as the state of user callbacks.
 *
 * @subsection tree_extraction Tree Generation
 *
 * The @ref GASparser contains a @ref GASparser::build_tree flag.  When
 * false, gas_parse will return NULL, and the parser will act as a scanner
 * only.  When true, gas_parse returns a tree.
 *
 * @subsection parsers Parser Callbacks
 *
 * The parser contains a number of callbacks.  When the function pointers are
 * NULL, the parser ignores them.
 *
 * The following callback allows for parse time pruning.
 * - @ref GASparser::on_pre_chunk
 *
 * The following are typical callbacks.
 *
 * - @ref GASparser::on_push_chunk
 * - @ref GASparser::on_pop_chunk
 *
 * The following fine grained callbacks are provided for any unforseen purposes.
 *
 * - @ref GASparser::on_push_id
 * - @ref GASparser::on_attribute
 * - @ref GASparser::on_payload
 * - @ref GASparser::on_pop_id
 *
 * @note Even when tree building is turned off, consider the following.  The
 * chunks provided in the push and pop callbacks will not contain children.
 * They will, however, contain a full list of parents, thus providing context.
 *
 * @section context File System Context
 *
 * Gas supports custom file systems.  Support is courtesty of @ref GAScontext
 * callbacks.
 *
 * @section pruning Tree Prunning
 *
 * Gas, like XML, is designed to be extensible.  Thus, it should be easy for an
 * application to ignore unsupported chunks.  One way of accomplishing this is
 * setting a @ref GAS_PRE_CHUNK function (in @ref GASparser::on_pre_chunk),
 * and returning false to ignore or prune a chunk (as well as any contained
 * children).
 *
 * @section scanning Scanning
 *
 * Consider a scenario where the goal is to quickly extract information from a
 * tree, but the payload content is not necessary.  The parser contains a flag,
 * GASparser::get_payloads, that is true by default.  When set to false, the
 * parser will seek over payload information, setting the payload pointers to
 * null.
 *
 * For the ultimate fast scan, set both GASparser::get_payloads and
 * GASparser::build_tree to false, and assign any necessary callbacks, such as
 * @ref GASparser::on_pre_chunk, @ref GASparser::on_push_chunk, and @ref
 * GASparser::on_pop_chunk.
 */
/*}}}*/

#include <gas/parser.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

GASunum encoded_size (GASunum value);
GASresult gas_read_encoded_num_parser (GASparser *p, GASunum *out);
GASresult gas_read_parser (GASparser *p, GASchunk **out);

/* gas_read_encoded_num_parser() {{{*/
GASresult gas_read_encoded_num_parser (GASparser *p, GASunum *out)
{
    GASunum retval;
    unsigned int bytes_read;
    GASunum i, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        p->context->read(p->handle, &byte, 1, &bytes_read,
                                  p->context->user_data);
        if (bytes_read != 1) {
            return GAS_ERR_FILE_EOF;
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
        p->context->read(p->handle, &byte, 1, &bytes_read,
                                  p->context->user_data);
        if (bytes_read != 1) {
            return GAS_ERR_FILE_EOF;
        }
        retval = (retval << 8) | byte;
    }
    *out = retval;
    return GAS_OK;
}
/*}}}*/
/* gas_read_parser() {{{*/

#define read_field(field)                                                   \
    do {                                                                    \
        result = gas_read_encoded_num_parser(p, &field##_size);             \
        if (result != GAS_OK) { goto abort; }                               \
        field = (GASubyte*)malloc(field##_size + 1);                        \
        p->context->read(p->handle, field, field##_size,                    \
                                  &bytes_read, p->context->user_data);      \
        ((GASubyte*)field)[field##_size] = 0;                               \
    } while (0)


/**
 * @brief Recursive context based gas parser.
 *
 * @warning Unlike other similar functions in the library, gas_read_parser is
 * intended for internal use only, via gas_parse().
 */
GASresult gas_read_parser (GASparser *p, GASchunk **out)
{
    GASresult result = GAS_OK;
    GASunum i;
    GASchunk* c = gas_new(NULL, 0);
    unsigned int bytes_read;
    GASbool cont;
    unsigned long jump = 0;

    result = gas_read_encoded_num_parser(p, &c->size);
    if (result != GAS_OK) { goto abort; }

/* id {{{*/
    read_field(c->id);

    if (p->on_pre_chunk) {
        cont = p->on_pre_chunk(c->id_size, c->id, p->context->user_data);
    } else {
        cont = GAS_TRUE;
    }
/*}}}*/

    if ( ! cont) {
        jump = c->size - encoded_size(c->id_size) - c->id_size;
        p->context->seek(p->handle, jump, SEEK_CUR, p->context->user_data);
        gas_destroy(c);
        *out = NULL;
        return GAS_OK;
    }

    if (p->on_push_id) {
        p->on_push_id(c->id_size, c->id, p->context->user_data);
    }

/* attributes {{{*/
    result = gas_read_encoded_num_parser(p, &c->nb_attributes);
    if (result != GAS_OK) { goto abort; }
    c->attributes = (GASattribute*)malloc(c->nb_attributes * sizeof(GASattribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
        if (p->on_attribute) {
            p->on_attribute(c->attributes[i].key_size, c->attributes[i].key,
                         c->attributes[i].value_size, c->attributes[i].value,
                         p->context->user_data);
        }
    }
/*}}}*/
/* payloads {{{*/
    if (p->get_payloads) {
        read_field(c->payload);
        if (p->on_payload) {
            p->on_payload(c->payload_size, c->payload, p->context->user_data);
        }
    } else {
        result = gas_read_encoded_num_parser(p, &c->payload_size);
        if (result != GAS_OK) { goto abort; }
        c->payload = NULL;
        jump = c->payload_size;
        p->context->seek(p->handle, jump,
                         SEEK_CUR,
                         p->context->user_data);
    }
/*}}}*/

    if (p->on_push_chunk) {
        p->on_push_chunk(c, p->context->user_data);
    }

/* children {{{*/
    result = gas_read_encoded_num_parser(p, &c->nb_children);
    if (result != GAS_OK) { goto abort; }
    c->children = (GASchunk**)malloc(c->nb_children * sizeof(GASchunk*));
    for (i = 0; i < c->nb_children; i++) {
        result = gas_read_parser(p, &c->children[i]);
        if (result != GAS_OK) { goto abort; }
        if (p->build_tree) {
            // if we are not building the tree, then the child will be null
            c->children[i]->parent = c;
        }
    }
/*}}}*/

    if (p->on_pop_chunk) {
        p->on_pop_chunk(c, p->context->user_data);
    }
    if (p->on_pop_id) {
        p->on_pop_id(c->id_size, c->id, p->context->user_data);
    }

    if (p->build_tree) {
        *out = c;
    } else {
        gas_destroy(c);
        *out = NULL;
    }
    return GAS_OK;

abort:
    gas_destroy(c);
    *out = NULL;
    return result;
}
/*}}}*/
/* parser routines {{{*/
GASparser* gas_parser_new (GAScontext* context)
{
    GASparser *p;

    p = (GASparser*)malloc(sizeof(GASparser));

    memset(p, 0, sizeof(GASparser));

    p->context = context;
    p->build_tree = GAS_TRUE;
    p->get_payloads = GAS_TRUE;

    return p;
}

void gas_parser_destroy (GASparser *p)
{
    free(p);
}

GASresult gas_parse (GASparser* p, const char *resource, GASchunk **out)
{
    GASresult result;
    GASchunk *c = NULL;
    GAScontext *s = p->context;

    result = s->open(resource, "rb", &p->handle, &s->user_data);
    if (result < GAS_OK) {
        return result;
    }
    result = gas_read_parser(p, &c);
    if (result < GAS_OK) {
        return result;
    }
    result = s->close(p->handle, s->user_data);

    *out = c;
    return result;
}
/*}}}*/

/* vim: set sw=4 fdm=marker : */
