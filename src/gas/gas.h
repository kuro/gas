
/**
 * @file gas.h
 * @brief gas definition
 */

#ifndef GAS_H
#define GAS_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


#if UNIX
#include <stdint.h>
#include <sys/types.h>
typedef uint8_t  GASubyte;
typedef int      GASenum;
typedef size_t   GASunum;
typedef ssize_t  GASnum;
#else
typedef unsigned char GASubyte;
typedef unsigned long GASunum;
typedef long          GASnum;
typedef int           GASenum;
#endif

typedef void     GASvoid;
typedef GASubyte GASbool;

enum
{
    GAS_FALSE,
    GAS_TRUE
};

/* types {{{*/
typedef struct _attribute attribute;
typedef struct _chunk chunk;
/* }}}*/

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
void gas_set_payload (chunk* c, GASunum payload_size, const void *payload);
/*GASunum gas_get_payload (chunk* c, void* payload);*/
/*@}*/
/** @defgroup children */
/*@{*/
void gas_add_child(chunk* parent, chunk* child);
GASunum gas_nb_children (chunk *c);
chunk* gas_get_child_at (chunk* c, GASunum index);
/*@}*/

void gas_update (chunk* c);
GASunum gas_total_size (chunk* c);


/*@}*/
/* }}}*/

/* attribute {{{*/
struct _attribute
{
    GASunum key_size;
    void *key;
    GASunum value_size;
    void *value;
};
/* }}}*/
/* chunk {{{*/
struct _chunk
{
    chunk* parent;

    GASunum size;

    GASunum id_size;
    void *id;

    GASunum nb_attributes;
    attribute* attributes;

    GASunum payload_size;
    void *payload;

    GASunum nb_children;
    chunk** children;
};
/* }}}*/

#ifdef __cplusplus
}
#endif


#endif  /* GAS_H defined */

/* vim: set sw=4 fdm=marker: */
