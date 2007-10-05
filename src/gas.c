
/**
 * @file gas.c
 * @brief gas implementation
 */

#include "gas.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


inline
size_t encoded_size (size_t value)
{
    return sizeof(size_t);
}


chunk* gas_new (size_t id_size, void *id)
{
    chunk *c;

    c = malloc(sizeof(chunk));
    assert(c != NULL);

    memset(c, 0, sizeof(chunk));

    c->id_size = id_size;
    c->id = id;

    return c;
}


chunk* gas_new_named (char *id)
{
    return gas_new(strlen(id), id);
}


/**
 * @brief Destroy a chunk tree.
 *
 * @note This does not release data for id, or the data contained in the
 * attributes, or the payload data.
 */
void gas_destroy (chunk* c)
{
    int i;
    // @note not freeing data contained by attributes
    free(c->attributes);
    for (i = 0; i < c->nb_children; i++) {
        gas_destroy(c->children[i]);
    }
    free(c->children);
    free(c);
}


void gas_set_id (chunk* c, size_t size, void *id)
{
    c->id_size = size;
    c->id = id;
}


void gas_set_attribute (chunk* c,
                              size_t key_size, void *key,
                              size_t value_size, void *value)
{
    c->nb_attributes++;

    attribute* tmp = realloc(c->attributes, c->nb_attributes*sizeof(attribute));
    assert(tmp);
    c->attributes = tmp;

    attribute *a = &c->attributes[c->nb_attributes-1];
    a->key_size = key_size;
    a->key = key;
    a->value_size = value_size;
    a->value = value;
}


void gas_set_attribute_string_pair(chunk* c,
                                         char *key,
                                         char *value)
{
    gas_set_attribute(c, strlen(key), key, strlen(value), value);
}


void gas_set_payload (chunk* c, size_t payload_size, void *payload)
{
    c->payload_size = payload_size;
    c->payload = payload;
}


void gas_add_child(chunk* parent, chunk* child)
{
    parent->nb_children++;

    chunk** tmp = realloc(parent->children, parent->nb_children*sizeof(chunk*));
    assert(tmp);
    parent->children = tmp;

    parent->children[parent->nb_children - 1] = child;
    child->parent = parent;
}


/**
 * @todo finish and test
 */
void gas_update (chunk* c)
{
    int i;

    size_t sum;

    sum = 0;
    sum += encoded_size(c->id_size);
    sum += c->id_size;

    for (i = 0; i < c->nb_attributes; i++) {
        sum += encoded_size(c->attributes[i].key_size);
        sum += c->attributes[i].key_size;
        sum += encoded_size(c->attributes[i].value_size);
        sum += c->attributes[i].value_size;
    }

    for (i = 0; i < c->nb_children; i++) {
        chunk* child = c->children[i];
        gas_update(child);
        sum += child->size;
    }

    printf("size: %ld\n", sum);
    c->size = sum;
    fflush(stdout);
}


/* temporary string based debugging {{{ */

int level = 1;
int level_iter;
#define indent() for (level_iter=0;level_iter<level;level_iter++) printf("  ")

void gas_print (chunk* c)
{
    int i;

    indent(); printf("chunk of size = %ld\n", c->size);
    indent(); printf("id of size %ld -> \"%s\"\n", c->id_size, (char*)c->id);
    indent(); printf("%ld attribute(s):\n", c->nb_attributes);
    for (i = 0; i < c->nb_attributes; i++) {
        indent(); printf(
            "%d -- \"%s\" (%ld) -> \"%s\" (%ld)\n",
            i,
            (char*)c->attributes[i].key, c->attributes[i].key_size,
            (char*)c->attributes[i].value, c->attributes[i].value_size
            );
    }
    indent(); printf("payload of size %ld:\n", c->payload_size);
    printf("---\n%s\n---\n", (char*)c->payload);

    level++;
    for (i = 0; i < c->nb_children; i++) {
        gas_print(c->children[i]);
    }
    level--;
}

/* }}} */

// vim: sw=4 fdm=marker
