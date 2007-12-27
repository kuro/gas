
/**
 * @file gas.h
 * @brief gas definition
 */

#ifndef GAS_H
#define GAS_H

#include <stdlib.h>

typedef unsigned char GASubyte;
typedef unsigned long int GASunum;
typedef long int     GASnum;
typedef int           GASenum;

typedef void     GASvoid;

#ifdef __cplusplus
typedef bool GASbool;
#define GAS_FALSE false
#define GAS_TRUE  true
#else
typedef GASubyte GASbool;
enum
{
    GAS_FALSE,
    GAS_TRUE
};
#endif

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
chunk* gas_new (GASunum id_size, const void *id);
chunk* gas_new_named (const char *id);
void gas_destroy (chunk* c);
/*@}*/
/* }}}*/
/* access {{{*/
/** @defgroup access */
/*@{*/
/** @defgroup id */
/*@{*/
void gas_set_id (chunk* c, GASunum size, const void *id);
GASnum gas_get_id (chunk* c, void* id, GASunum offset, GASunum limit);
void gas_set_id_s (chunk* c, const char* id);
char* gas_get_id_s (chunk* c);
/*@}*/
/** @defgroup attribute */
/*@{*/
GASnum gas_index_of_attribute (chunk* c, GASunum key_size,const void* key);
void gas_set_attribute (chunk* c,
                        GASunum key_size, const void *key,
                        GASunum value_size, const void *value);
int gas_has_attribute (chunk* c, GASunum key_size, void* key);
GASnum gas_attribute_value_size (chunk* c, GASunum index);
GASnum gas_get_attribute (chunk* c, GASunum index,
                          void* value, GASunum offset, GASunum limit);
/*@}*/
/** @defgroup payload */
/*@{*/
void gas_set_payload (chunk* c, GASunum payload_size, const GASvoid *payload);
/*GASunum gas_get_payload (chunk* c, void* payload);*/
/*@}*/
/** @defgroup children */
/*@{*/
chunk* gas_get_parent(chunk* c);
void gas_add_child(chunk* parent, chunk* child);
GASunum gas_nb_children (chunk *c);
chunk* gas_get_child_at (chunk* c, GASunum index);
/*@}*/

void gas_update (chunk* c);
GASunum gas_total_size (chunk* c);


/*@}*/
/* }}}*/

#ifdef __cplusplus
} // extern C
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

/* chunk {{{*/
struct Chunk
{
    chunk* parent;

    GASunum size;

    GASunum id_size;
    void *id;

    GASunum nb_attributes;
    struct Attribute* attributes;

    GASunum payload_size;
    GASubyte *payload;

    GASunum nb_children;
    chunk** children;

#ifdef __cplusplus
public:
    //static void* operator new (size_t size); 
    //static void operator delete (void *p);

    inline Chunk (const char *id = NULL);
    inline Chunk (GASunum id_size, const GASvoid *id);
    inline ~Chunk();

    inline void add_child (Chunk* child) { gas_add_child(this, child); }

    inline void set_attribute (const char* key, const char* val);
    inline void set_attribute (const char* key,       char* val);
    inline void set_attribute (      char* key, const char* val);
    inline void set_attribute (      char* key,       char* val);

    template<typename V>
    inline void set_attribute (const char* key, const V& val);
    template<typename V>
    inline void set_attribute (      char* key, const V& val);

    template<typename K, typename V>
    inline void set_attribute (const K& key, const V& val);

    template<typename V>
    inline void get_attribute (const char* key, V& retval);

    template<typename K, typename V>
    inline void get_attribute (const K& key, V& retval);

    inline void set_payload (GASunum size, const GASvoid *payload);

#endif
};

#ifdef __cplusplus
#include <gas/gas.inl>
#endif


/* }}}*/

#endif  /* GAS_H defined */

/* vim: set sw=4 fdm=marker: */
