
/**
 * @file parser.h
 * @brief parser definition
 */

#ifndef GAS_PARSER_H
#define GAS_PARSER_H

#include <gas/session.h>

typedef struct _gas_parser gas_parser;

typedef GASbool (*GAS_PRE_CHUNK)    (size_t id_size, void *id, void *user_data);
typedef GASvoid (*GAS_PUSH_ID)      (size_t id_size, void *id, void *user_data);
typedef GASvoid (*GAS_PUSH_CHUNK)   (chunk* c, void *user_data);
typedef GASvoid (*GAS_ON_ATTRIBUTE) (size_t key_size, void *key,
                                     size_t value_size, void *value,
                                     void *user_data);
typedef GASvoid (*GAS_ON_PAYLOAD)   (size_t payload_size, void *payload, void *user_data);
typedef GASvoid (*GAS_POP_ID)       (size_t id_size, void *id, void *user_data);
typedef GASvoid (*GAS_POP_CHUNK)    (chunk* c, void *user_data);

struct _gas_parser
{
    gas_session* session;
    void *handle;
    GASbool build_tree;

    GAS_PRE_CHUNK    on_pre_chunk;
    GAS_PUSH_ID      on_push_id;
    GAS_PUSH_CHUNK   on_push_chunk;
    GAS_ON_ATTRIBUTE on_attribute;
    GAS_ON_PAYLOAD   on_payload;
    GAS_POP_ID       on_pop_id;
    GAS_POP_CHUNK    on_pop_chunk;
};

gas_parser* gas_parser_new (gas_session* session, GASbool build_tree);
chunk* gas_parse (gas_parser* p, const char *resource);
void gas_parser_destroy (gas_parser *p);

#endif /* GAS_PARSER_H defined */

/* vim: set sw=4 fdm=marker :*/
