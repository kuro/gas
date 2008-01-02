
/**
 * @file tree.h
 * @brief tree definition
 */

#ifndef GAS_TREE_H
#define GAS_TREE_H

#include <gas/types.h>

/* types {{{*/
typedef struct Attribute attribute;
typedef struct Chunk chunk;
/* }}}*/

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif

/* construction {{{*/
/** @defgroup construction */
/*@{*/
chunk* gas_new (GASunum id_size, const GASvoid *id);
chunk* gas_new_named (const GASchar *id);
GASvoid gas_destroy (chunk* c);
/*@}*/
/* }}}*/
/* access {{{*/
/** @defgroup access */
/*@{*/
/** @defgroup id */
/*@{*/
GASvoid gas_set_id (chunk* c, const GASvoid *id, GASunum size);
GASnum gas_get_id (chunk* c, GASvoid* id, GASunum limit);
GASvoid gas_set_id_s (chunk* c, const GASchar* id);
GASchar* gas_get_id_s (chunk* c);
/*@}*/
/** @defgroup attribute */
/*@{*/
GASnum gas_index_of_attribute (chunk* c, const GASvoid* key, GASunum key_size);
GASvoid gas_set_attribute (chunk* c,
                        const GASvoid *key, GASunum key_size,
                        const GASvoid *value, GASunum value_size);
GASbool gas_has_attribute (chunk* c, GASvoid* key, GASunum key_size);
GASnum gas_attribute_value_size (chunk* c, GASunum index);
GASnum gas_get_attribute (chunk* c, GASunum index, GASvoid* value, GASunum limit);
/*@}*/
/** @defgroup payload */
/*@{*/
GASvoid gas_set_payload (chunk* c, const GASvoid *payload, GASunum payload_size);
GASunum gas_get_payload (chunk* c, GASvoid* payload, GASunum limit);
/*@}*/
/** @defgroup children */
/*@{*/
chunk* gas_get_parent(chunk* c);
GASvoid gas_add_child(chunk* parent, chunk* child);
GASunum gas_nb_children (chunk *c);
chunk* gas_get_child_at (chunk* c, GASunum index);
/*@}*/

GASvoid gas_update (chunk* c);
GASunum gas_total_size (chunk* c);


/*@}*/
/* }}}*/

#ifdef __cplusplus
} /* extern C */
#endif

/* attribute {{{*/
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
    chunk* parent;

    GASunum size;

    GASunum id_size;
    GASubyte *id;

    GASunum nb_attributes;
    struct Attribute* attributes;

    GASunum payload_size;
    GASubyte *payload;

    GASunum nb_children;
    chunk** children;

#ifdef __cplusplus
public:
    /*static GASvoid* operator new (size_t size); */
    /*static GASvoid operator delete (GASvoid *p);*/

    inline Chunk (const GASchar *id = NULL);
    inline Chunk (GASunum id_size, const GASvoid *id);
    inline ~Chunk();

    inline GASvoid add_child (Chunk* child);

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

    inline GASvoid set_payload (const GASvoid *payload, GASunum size);

#endif
};

#ifdef __cplusplus
#include <gas/tree.inl>
#endif


/* }}}*/

#endif /* GAS_TREE_H defined */

/* vim: set sw=4 fdm=marker :*/
