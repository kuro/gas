
/**
 * @file gas.h
 * @brief gas definition
 */

#pragma once

#include <stdlib.h>

/* types {{{*/
typedef struct _attribute attribute;
typedef struct _chunk chunk;
/* }}}*/

/* construction {{{*/
chunk* gas_new (size_t id_size, const void *id);
chunk* gas_new_named (const char *id);
void gas_destroy (chunk* c);
/* }}}*/
/* access {{{*/
void gas_set_id (chunk* c, size_t size, const void *id);
char* gas_get_id_as_string (chunk* c);
void gas_set_attribute (chunk* c,
                        size_t key_size, const void *key,
                        size_t value_size, const void *value);
void gas_set_payload (chunk* c, size_t payload_size, const void *payload);
char* gas_get_payload_as_string (chunk* c);
void gas_update (chunk* c);

void gas_set_attribute_string_pair(chunk* c, const char *key, const char *value);
int gas_has_attribute (chunk* c, size_t key_size, void* key);
int gas_get_attribute (chunk* c,
                        size_t key_size, const void* key,
                        size_t* value_size, void** value);
char* gas_get_attribute_string_pair (chunk* c, const char* key);

void gas_add_child(chunk* parent, chunk* child);
size_t gas_nb_children (chunk *c);
chunk* gas_get_child_at (chunk* c, size_t index);
/* }}}*/
/* io {{{*/
void gas_write (chunk* self, int fd);
void gas_write_encoded_num (int fd, size_t value);
chunk* gas_read (int fd);
/* }}}*/

/* attribute {{{*/
struct _attribute
{
    size_t key_size;
    void *key;
    size_t value_size;
    void *value;
};
/* }}}*/
/* chunk {{{*/
struct _chunk
{
    chunk* parent;

    size_t size;

    size_t id_size;
    void *id;

    size_t nb_attributes;
    attribute* attributes;

    size_t payload_size;
    void *payload;

    size_t nb_children;
    chunk** children;
};
/* }}}*/

/* vim: sw=4 fdm=marker: */
