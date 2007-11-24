
/**
 * @file parser.h
 * @brief parser definition
 */

#ifndef GAS_PARSER_H
#define GAS_PARSER_H

#include <gas/context.h>

typedef struct _gas_parser gas_parser;

/**
 * @brief Previews a chunk by providing the id, and provides an early out if the chunk is not desired.
 *
 * When false is returned, the chunk will be pruned (seeked over) from the tree
 * (practically ignored).
 */
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
    gas_context* context;
    void *handle;

    /**
     * @brief Determines whether or not a tree is built.
     *
     * By default, build_tree is true.  However, when false, the parser will
     * not build the tree, but call the callbacks instead.  Of course, it is
     * pointless to set this to false unless callbacks are provided.
     */
    GASbool build_tree;
    GASbool get_payloads;

    GAS_PRE_CHUNK    on_pre_chunk;
    GAS_PUSH_ID      on_push_id;
    GAS_PUSH_CHUNK   on_push_chunk;
    GAS_ON_ATTRIBUTE on_attribute;
    GAS_ON_PAYLOAD   on_payload;
    GAS_POP_ID       on_pop_id;
    GAS_POP_CHUNK    on_pop_chunk;
};

/**
 * @param build_tree When true, return a parsed tree.  When false, scan the
 * tree only, invoking any set callbacks in the process.
 *
 * @see gas_parser::build_tree
 */
gas_parser* gas_parser_new (gas_context* context, GASbool build_tree);
chunk* gas_parse (gas_parser* p, const char *resource);
void gas_parser_destroy (gas_parser *p);

#endif /* GAS_PARSER_H defined */

/* vim: set sw=4 fdm=marker :*/
