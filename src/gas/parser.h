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

/**
 * @file parser.h
 * @brief parser definition
 */

#include <gas/context.h>
#include <gas/tree.h>

#ifndef GAS_PARSER_H
#define GAS_PARSER_H

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/**
 * @brief Previews a chunk by providing the id, and provides an early out if
 * the chunk is not desired.
 *
 * When false is returned, the chunk will be pruned (seeked over) from the tree
 * (practically ignored).
 */
typedef GASbool (*GAS_PRE_CHUNK)    (GASunum id_size, void *id, void *user_data);
typedef GASvoid (*GAS_PUSH_ID)      (GASunum id_size, void *id, void *user_data);
typedef GASvoid (*GAS_PUSH_CHUNK)   (GASchunk* c, void *user_data);
typedef GASvoid (*GAS_ON_ATTRIBUTE) (GASunum key_size, void *key,
                                     GASunum value_size, void *value,
                                     void *user_data);
typedef GASvoid (*GAS_ON_PAYLOAD)   (GASunum payload_size, void *payload, void *user_data);
typedef GASvoid (*GAS_POP_ID)       (GASunum id_size, void *id, void *user_data);
typedef GASvoid (*GAS_POP_CHUNK)    (GASchunk* c, void *user_data);

typedef struct
{
    GAScontext* context;
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
} GASparser;

GASparser* gas_parser_new (GAScontext* context);
void gas_parser_destroy (GASparser *p);

GASresult gas_read_parser (GASparser *p, GASchunk **out);

GASresult gas_parse (GASparser* p, const char *resource, GASchunk **out);

#ifdef __cplusplus
}
#endif


#endif /* GAS_PARSER_H defined */

/* vim: set sw=4 fdm=marker :*/
