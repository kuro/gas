
/**
 * @file gas.h
 * @brief gas definition
 */

#pragma once

#include <linux/types.h>

typedef struct _attribute attribute;
typedef struct _chunk chunk;

struct _attribute
{
    size_t key_size;
    void *key;
    size_t value_size;
    void *value;
};


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

chunk* gas_new (size_t id_size, void *id);
chunk* gas_new_named (char *id);
void gas_destroy (chunk* c);

void gas_set_id (chunk* c, size_t size, void *id);
void gas_set_attribute (chunk* c,
                        size_t key_size, void *key,
                        size_t value_size, void *value);
void gas_set_payload (chunk* c, size_t payload_size, void *payload);
void gas_update (chunk* c);
void gas_set_attribute_string_pair(chunk* c, char *key, char *value);
void gas_add_child(chunk* parent, chunk* child);

void gas_write (chunk* self, int fd);
void gas_write_encoded_num (int fd, size_t value);
chunk* gas_read (int fd);

// vim: sw=4 fdm=marker
