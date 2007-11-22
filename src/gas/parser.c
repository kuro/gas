
#include "parser.h"
#include "ntstring.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>

/* default callbacks {{{*/
GASbool gas_default_pre_chunk (size_t id_size, void *id, void *user_data)
{
    return GAS_TRUE;
}

void gas_default_push_id (size_t id_size, void *id, void *user_data)
{
    /*printf("push_chunk: id_size=%ld id=%s\n", id_size, (char*)id);*/
}

void gas_default_push_chunk (chunk* c, void *user_data)
{
    /*gas_print(c);*/
}

void gas_default_pop_id (size_t id_size, void *id, void *user_data)
{
    /*printf("pop_chunk: id_size=%ld id=%s\n", id_size, (char*)id);*/
}

void gas_default_pop_chunk (chunk* c, void *user_data)
{
}

void gas_default_on_attribute (size_t key_size, void *key,
                   size_t value_size, void *value,
                   void *user_data)
{
    /*printf("attribute: key=%s value=%s\n", (char*)key, (char*)value);*/
}

void gas_default_on_payload (size_t payload_size, void *payload, void *user_data)
{
    /*printf("payload: payload=%s\n", (char*)payload);*/
}
/*}}}*/
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
        p->session->read_callback(p->handle, &byte, 1, &bytes_read,
                                  p->session->user_data);
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
        p->session->read_callback(p->handle, &byte, 1, &bytes_read,
                                  p->session->user_data);
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
        p->session->read_callback(p->handle, field, field##_size,             \
                                  &bytes_read, p->session->user_data);        \
        ((GASubyte*)field)[field##_size] = 0;                                 \
    } while (0)

GASunum encoded_size (GASunum value);

chunk* gas_read_parser (gas_parser *p)
{
    int i;
    chunk* c = gas_new(0, NULL);
    unsigned int bytes_read;
    GASbool cont;

    c->size = gas_read_encoded_num_parser(p);
    read_field(c->id);

    cont = p->on_pre_chunk(c->id_size, c->id, p->session->user_data);
    if ( ! cont) {
        GASunum jump = c->size - encoded_size(c->id_size) - c->id_size;
        p->session->seek_callback(p->handle, jump, p->session->user_data);
        gas_destroy(c);
        return NULL;
    }

    p->on_push_id(c->id_size, c->id, p->session->user_data);

    c->nb_attributes = gas_read_encoded_num_parser(p);
    c->attributes = malloc(c->nb_attributes * sizeof(attribute));
    for (i = 0; i < c->nb_attributes; i++) {
        read_field(c->attributes[i].key);
        read_field(c->attributes[i].value);
        p->on_attribute(c->attributes[i].key_size, c->attributes[i].key,
                     c->attributes[i].value_size, c->attributes[i].value,
                     p->session->user_data);
    }
    read_field(c->payload);
    p->on_payload(c->payload_size, c->payload, p->session->user_data);

    p->on_push_chunk(c, p->session->user_data);

    c->nb_children = gas_read_encoded_num_parser(p);
    c->children = malloc(c->nb_children * sizeof(chunk*));
    for (i = 0; i < c->nb_children; i++) {
        c->children[i] = gas_read_parser(p);
    }
    p->on_pop_chunk(c, p->session->user_data);
    p->on_pop_id(c->id_size, c->id, p->session->user_data);

    if (p->build_tree) {
        return c;
    } else {
        gas_destroy(c);
        return NULL;
    }
}
/*}}}*/
/* parser routines {{{*/
gas_parser* gas_parser_new (gas_session* session, GASbool build_tree)
{
    gas_parser *p;

    p = malloc(sizeof(gas_parser));

    memset(p, 0, sizeof(gas_parser));

    p->session = session;
    p->build_tree = build_tree;

    p->on_pre_chunk   = gas_default_pre_chunk;
    p->on_push_id     = gas_default_push_id;    
    p->on_push_chunk  = gas_default_push_chunk; 
    p->on_attribute   = gas_default_on_attribute;  
    p->on_payload     = gas_default_on_payload;    
    p->on_pop_id      = gas_default_pop_id;     
    p->on_pop_chunk   = gas_default_pop_chunk;  

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
    gas_session *s = p->session;

    status = s->open_callback(resource, "rb", &p->handle, &s->user_data);
    c = gas_read_parser(p);
    status = s->close_callback(p->handle, s->user_data);

    return c;
}
/*}}}*/

/* vim: set sw=4 fdm=marker : */
