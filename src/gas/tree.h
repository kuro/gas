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
 * @file tree.h
 * @brief tree definition
 */

#ifndef GAS_TREE_H
#define GAS_TREE_H

#include <gas/types.h>


#if defined(GAS_ENABLE_CPP) && defined(__cplusplus)
#include <exception>
namespace Gas
{
/*}*/
#endif

/* GASattribute {{{*/
struct Attribute
{
    GASunum key_size;
    GASubyte *key;
    GASunum value_size;
    GASubyte *value;
};
/* }}}*/

/* Chunk {{{*/
struct Chunk
{
    struct Chunk* parent;

    GASunum size;

    GASunum id_size;
    GASubyte *id;

    GASunum nb_attributes;
    struct Attribute* attributes;

    GASunum payload_size;
    GASubyte *payload;

    GASunum nb_children;
    struct Chunk** children;

#if defined(GAS_ENABLE_CPP) && defined(__cplusplus)
public:

    inline Chunk (const GASchar *id = NULL);
    inline Chunk (GASunum id_size, const GASvoid *id);
    inline ~Chunk();

/* attributes {{{*/
    inline GASvoid set_attribute (const GASchar* key, const GASchar* val);
    inline GASvoid set_attribute (const GASchar* key,       GASchar* val);
    inline GASvoid set_attribute (      GASchar* key, const GASchar* val);
    inline GASvoid set_attribute (      GASchar* key,       GASchar* val);

    template<typename V>
    inline GASvoid set_attribute (const GASchar* key, const V& val);

    template<typename V>
    inline GASvoid set_attribute (      GASchar* key, const V& val);

    template<typename K, typename V>
    inline GASvoid set_attribute (const K& key, const V& val);

    template<typename V>
    inline GASvoid get_attribute (const GASchar* key, V& retval);

    template<typename K, typename V>
    inline GASvoid get_attribute (const K& key, V& retval);
/*}}}*/

    inline GASvoid set_payload (const GASvoid *payload, GASunum size);

    inline Chunk* add_child (Chunk* child);

    inline Chunk* operator<< (Chunk* child);

#endif
};

#if defined(GAS_ENABLE_CPP) && defined(__cplusplus)

class Exception : public std::exception
{
protected:
    char message[128];

public:
    inline Exception (char *message) throw();
    inline virtual char* what () throw();
};

}  /* namespace Gas */
#endif


/* }}}*/

#if defined(GAS_ENABLE_CPP) && defined(__cplusplus)
typedef Gas::Attribute GASattribute;
typedef Gas::Chunk GASchunk;
#else
typedef struct Attribute GASattribute;
typedef struct Chunk GASchunk;
#endif


/* C Functions {{{*/
#ifdef __cplusplus
extern "C"
{
/*}*/
#endif

int gas_cmp(const GASubyte *a, GASunum a_len, const GASubyte *b, GASunum b_len);
GASchar* gas_error_string (GASresult result);
void gas_hexdump (GASvoid *input, GASunum size);

/**
 * @defgroup io IO
 */

/**
 * @defgroup access Tree Access
 */
/* construction {{{*/
/**
 * @defgroup construction Tree Construction/Destruction
 * @ingroup access
 */
/*@{*/
GASchunk* gas_new (const GASvoid *id, GASunum id_size);
GASchunk* gas_new_named (const GASchar *id);
GASvoid gas_destroy (GASchunk* c);
/*@}*/
/* }}}*/
/* access {{{*/
/**
 * @defgroup id ID Access
 * @ingroup access
 */
/*@{*/
GASvoid gas_set_id (GASchunk* c, const GASvoid *id, GASunum size);
GASnum gas_get_id (GASchunk* c, GASvoid* id, GASunum limit);
GASvoid gas_set_id_s (GASchunk* c, const GASchar* id);
GASchar* gas_get_id_s (GASchunk* c);
GASunum gas_id_size (GASchunk* c);
/*@}*/
/**
 * @defgroup attribute  Attribute Access
 * @ingroup access
 */
/*@{*/
GASnum gas_index_of_attribute (GASchunk* c, const GASvoid* key, GASunum key_size);
GASvoid gas_set_attribute (GASchunk* c,
                        const GASvoid *key, GASunum key_size,
                        const GASvoid *value, GASunum value_size);
GASbool gas_has_attribute (GASchunk* c, GASvoid* key, GASunum key_size);
GASnum gas_attribute_value_size (GASchunk* c, GASunum index);
GASnum gas_get_attribute (GASchunk* c, GASunum index, GASvoid* value, GASunum limit);
GASnum gas_delete_attribute_at (GASchunk* c, GASunum index);
GASnum gas_delete_child_at (GASchunk* c, GASunum index);
/*@}*/
/**
 * @defgroup payload Payload Access
 * @ingroup access
 */
/*@{*/
GASvoid gas_set_payload (GASchunk* c, const GASvoid *payload, GASunum payload_size);
GASnum gas_get_payload (GASchunk* c, GASvoid* payload, GASunum limit);
GASunum gas_payload_size (GASchunk* c);
/*@}*/
/**
 * @defgroup children  Child Access
 * @ingroup access
 */
/*@{*/
GASchunk* gas_get_parent(GASchunk* c);
GASvoid gas_add_child(GASchunk* parent, GASchunk* child);
GASunum gas_nb_children (GASchunk *c);
GASchunk* gas_get_child_at (GASchunk* c, GASunum index);
/*@}*/

GASvoid gas_update (GASchunk* c);
GASunum gas_total_size (GASchunk* c);


/* }}}*/

#ifdef __cplusplus
} /* extern C */
#endif
/*}}}*/

#if defined(GAS_ENABLE_CPP) && defined(__cplusplus)
#include <gas/tree.inl>
#endif

#endif /* GAS_TREE_H defined */

/* vim: set sw=4 fdm=marker :*/
