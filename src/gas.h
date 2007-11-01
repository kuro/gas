
/**
 * @file gas.h
 * @brief gas definition
 */

#ifndef GAS_H
#define GAS_H

#include <stdlib.h>

typedef unsigned char GASubyte;
typedef unsigned long GASunum;
typedef long GASnum;
typedef int GASenum;

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
chunk* gas_new (GASunum id_size, const void *id);
chunk* gas_new_named (const char *id);
void gas_destroy (chunk* c);
/* }}}*/
/* access {{{*/
void gas_set_id (chunk* c, GASunum size, const void *id);
char* gas_get_id_s (chunk* c);
void gas_set_attribute (chunk* c,
                        GASunum key_size, const void *key,
                        GASunum value_size, const void *value);
void gas_set_payload (chunk* c, GASunum payload_size, const void *payload);
void gas_set_payload_s (chunk* c, const char* payload);
/*GASunum gas_get_payload (chunk* c, void* payload);*/
char* gas_get_payload_s (chunk* c);
void gas_update (chunk* c);
GASunum gas_total_size (chunk* c);

void gas_set_attribute_s (chunk* c,
                          const char *key,
                          GASunum value_size, const void *value);
void gas_set_attribute_ss(chunk* c, const char *key, const char *value);
int gas_has_attribute (chunk* c, GASunum key_size, void* key);
int gas_get_attribute (chunk* c,
                       GASunum key_size, const void* key,
                       GASunum* value_size, void* value);
int gas_get_attribute_s (chunk* c,
                         const char* key,
                         GASunum* value_size, void* value);
char* gas_get_attribute_ss (chunk* c, const char* key);

void gas_add_child(chunk* parent, chunk* child);
GASunum gas_nb_children (chunk *c);
chunk* gas_get_child_at (chunk* c, GASunum index);
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

/* fd io {{{*/
void gas_write_fd (chunk* self, int fd);
void gas_write_encoded_num_fd (int fd, GASunum value);
chunk* gas_read_fd (int fd);

void gas_write_encoded_num_fd (int fd, GASunum value);
GASunum gas_read_encoded_num_fd (int fd);
/*}}}*/
/* buffer io {{{*/
long gas_buf_write_encoded_num (GASubyte* buf, GASunum value);
long gas_buf_write (chunk* self, GASubyte* buf);
/*}}}*/

#endif  /* GAS_H defined */

/* vim: set sw=4 fdm=marker: */
