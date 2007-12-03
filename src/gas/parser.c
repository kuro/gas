
/**
 * @page parser The Gas Parser
 *
 * Of all the gas input/output methods, the Gas Parser is the most powerful and
 * flexible.
 *
 * @section parser_tuning Parser Tuning
 *
 * The parser's behavior is determined based upon a few factors, including the
 * seetings gas_parser::build_tree, as well as the state of user callbacks.
 *
 * @subsection tree_extraction Tree Generation
 *
 * The @ref gas_parser contains a @ref gas_parser::build_tree flag.  When
 * false, gas_parse will return NULL, and the parser will act as a scanner
 * only.  When true, gas_parse returns a tree.
 *
 * @subsection parsers Parser Callbacks
 *
 * The parser contains a number of callbacks.  When the function pointers are
 * NULL, the parser ignores them.
 *
 * The following callback allows for parse time pruning.
 * - @ref gas_parser::on_pre_chunk
 *
 * The following are typical callbacks.
 *
 * - @ref gas_parser::on_push_chunk
 * - @ref gas_parser::on_pop_chunk
 *
 * The following fine grained callbacks are provided for any unforseen purposes.
 *
 * - @ref gas_parser::on_push_id
 * - @ref gas_parser::on_attribute
 * - @ref gas_parser::on_payload
 * - @ref gas_parser::on_pop_id
 *
 * @note Even when tree building is turned off, consider the following.  The
 * chunks provided in the push and pop callbacks will not contain children.
 * They will, however, contain a full list of parents, thus providing context.
 *
 * @section context File System Context
 *
 * Gas supports custom file systems.  Support is courtesty of @ref gas_context
 * callbacks.
 *
 * @section pruning Tree Prunning
 *
 * Gas, like XML, is designed to be extensible.  Thus, it should be easy for an
 * application to ignore unsupported chunks.  One way of accomplishing this is
 * setting a @ref GAS_PRE_CHUNK function (in @ref gas_parser::on_pre_chunk),
 * and returning false to ignore or prune a chunk (as well as any contained
 * children).
 *
 * @section scanning Scanning
 *
 * Consider a scenario where the goal is to quickly extract information from a
 * tree, but the payload content is not necessary.  The parser contains a flag,
 * gas_parser::get_payloads, that is true by default.  When set to false, the
 * parser will seek over payload information, setting the payload pointers to
 * null.
 *
 * For the ultimate fast scan, set both gas_parser::get_payloads and
 * gas_parser::build_tree to false, and assign any necessary callbacks, such as
 * @ref gas_parser::on_pre_chunk, @ref gas_parser::on_push_chunk, and @ref
 * gas_parser::on_pop_chunk.
 */

#include "parser.h"
#include "ntstring.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>

/* gas_read_encoded_num_parser() {{{*/
GASunum gas_read_encoded_num_parser (gas_parser *p)
{
    GASunum retval;
    unsigned int bytes_read;
    int i, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;
    GASunum additional_bytes_to_read;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        /*bytes_read = read(fd, &byte, 1);*/
        p->context->read(p->handle, &byte, 1, &bytes_read,
                                  p->context->user_data);
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
        /*bytes_read = read(fd, &byte, 1);*/
        p->context->read(p->handle, &byte, 1, &bytes_read,
                                  p->context->user_data);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        retval = (retval << 8) | byte;
    }
    return retval;
}
/*}}}*/
/* gas_read_parser() {{{*/

#define read_field(field)                                                     \
    do {                                                                      \
        field##_size = gas_read_encoded_num_parser(p);                        \
        field = malloc(field##_size + 1);                                     \
        p->context->read(p->handle, field, field##_size,             \
                                  &bytes_read, p->context->user_data);        \
        ((GASubyte*)field)[field##_size] = 0;                                 \
    } while (0)

GASunum encoded_size (GASunum value);

/**
 * @brief Recursive context based gas parser.
 *
 * @warning Unlike other similar functions in the library, gas_read_parser is
 * intended for internal use only, via gas_parse().
 */
chunk* gas_read_parser (gas_parser *p)
{
    int i;
    chunk* c = gas_new(0, NULL);
    unsigned int bytes_read;
    GASbool cont;

    c->size = gas_read_encoded_num_parser(p);
    read_field(c->id);

    if (p->on_pre_chunk) {
        cont = p->on_pre_chunk(c->id_size, c->id, p->context->user_data);
    } else {
        cont = GAS_TRUE;
    }
    if ( ! cont) {
        GASunum jump = c->size - encoded_size(c->id_size) - c->id_size;
        p->context->seek(p->handle, jump, p->context->user_data);
        gas_destroy(c);
        return NULL;
    }

    if (p->on_push_id) {
        p->on_push_id(c->id_size, c->id, p->context->user_data);
    }

    c->nb_attributes = gas_read_encoded_num_parser(p);
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
        if (p->on_attribute) {
            p->on_attribute(c->attributes[i].key_size, c->attributes[i].key,
                         c->attributes[i].value_size, c->attributes[i].value,
                         p->context->user_data);
        }
    }

    if (p->get_payloads) {
        read_field(c->payload);
        if (p->on_payload) {
            p->on_payload(c->payload_size, c->payload, p->context->user_data);
        }
    } else {
        c->payload_size = gas_read_encoded_num_parser(p);
        c->payload = NULL;
        p->context->seek(p->handle, c->payload_size,
                                  p->context->user_data);
    }

    if (p->on_push_chunk) {
        p->on_push_chunk(c, p->context->user_data);
    }

    c->nb_children = gas_read_encoded_num_parser(p);
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        c->children[i] = gas_read_parser(p);
        c->children[i]->parent = c;
    }
    if (p->on_pop_chunk) {
        p->on_pop_chunk(c, p->context->user_data);
    }
    if (p->on_pop_id) {
        p->on_pop_id(c->id_size, c->id, p->context->user_data);
    }

    if (p->build_tree) {
        return c;
    } else {
        gas_destroy(c);
        return NULL;
    }
}
/*}}}*/
/* parser routines {{{*/
gas_parser* gas_parser_new (gas_context* context)
{
    gas_parser *p;

    p = malloc(sizeof(gas_parser));

    memset(p, 0, sizeof(gas_parser));

    p->context = context;
    p->build_tree = GAS_TRUE;
    p->get_payloads = GAS_TRUE;

#if 0
    p->on_pre_chunk   = gas_default_pre_chunk;
    p->on_push_id     = gas_default_push_id;    
    p->on_push_chunk  = gas_default_push_chunk; 
    p->on_attribute   = gas_default_on_attribute;  
    p->on_payload     = gas_default_on_payload;    
    p->on_pop_id      = gas_default_pop_id;     
    p->on_pop_chunk   = gas_default_pop_chunk;  
#endif

    return p;
}

void gas_parser_destroy (gas_parser *p)
{
    free(p);
}

chunk* gas_parse (gas_parser* p, const char *resource)
{
    chunk *c = NULL;
    GASnum status;
    gas_context *s = p->context;

    status = s->open(resource, "rb", &p->handle, &s->user_data);
    c = gas_read_parser(p);
    status = s->close(p->handle, s->user_data);

    return c;
}
/*}}}*/

/* vim: set sw=4 fdm=marker : */
