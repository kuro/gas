
/**
 * @file gas.inl
 * @brief gas definition
 */

#pragma once

#include <string.h>
#include <iostream>

/* macro copy_to_field() {{{*/
#define copy_to_field(field)                                                \
    do {                                                                    \
        this->field##_size = field##_size;                                  \
        this->field = (GASubyte*)realloc(this->field, field##_size + 1);    \
        memcpy(this->field, field, field##_size);                           \
        ((GASubyte*)this->field)[field##_size] = 0;                         \
    } while (0)
/*}}}*/
inline Chunk::Chunk (GASunum id_size, const GASvoid *id)
{
    memset(this, 0, sizeof(Chunk));
    if (id) {
        copy_to_field(id);
    }
}
inline Chunk::Chunk (const char *id)
{
    GASunum id_size = strlen(id);
    memset(this, 0, sizeof(Chunk));
    if (id) {
        copy_to_field(id);
    }
}
inline Chunk::~Chunk ()
{
    GASunum i;
    free(id);
    for (i = 0; i < nb_attributes; i++) {
        free(attributes[i].key);
        free(attributes[i].value);
    }
    free(attributes);
    free(payload);
    for (i = 0; i < nb_children; i++) {
        delete children[i];
    }
    free(children);
}

template<typename V>
inline void Chunk::set_attribute (const char* key, const V& val)
{
    gas_set_attribute(this, strlen(key), key, sizeof(V), &val);
}

template<typename V>
inline void Chunk::set_attribute (char* key, const V& val)
{
    gas_set_attribute(this, strlen(key), key, sizeof(V), &val);
}

inline void Chunk::set_attribute (const char* key, const char* val)
{
    gas_set_attribute(this, strlen(key), key, strlen(val), val);
}

inline void Chunk::set_attribute (char* key, const char* val)
{
    gas_set_attribute(this, strlen(key), key, strlen(val), val);
}

inline void Chunk::set_attribute (const char* key, char* val)
{
    gas_set_attribute(this, strlen(key), key, strlen(val), val);
}

inline void Chunk::set_attribute (char* key, char* val)
{
    gas_set_attribute(this, strlen(key), key, strlen(val), val);
}

template<typename K, typename V>
inline void Chunk::set_attribute (const K& key, const V& val)
{
    //std::clog << typeid(K).name() << std::endl;
    std::clog << typeid(key).name() << std::endl;
    std::clog << typeid(typeof(key)).name() << std::endl;
    std::clog << (typeid(key).name() == typeid(char*).name()) << std::endl;
    std::clog << (1 xor 1) << std::endl;
    gas_set_attribute(this, sizeof(K), &key, sizeof(V), &val);
}

template<typename V>
inline void Chunk::get_attribute (const char* key, V& retval)
{
    gas_get_attribute_s(this, key, &retval, 0, sizeof(retval));
}

template<typename K, typename V>
inline void Chunk::get_attribute (const K& key, V& retval)
{
    puts(";)");
    GASnum index;
    index = gas_index_of_attribute(this, sizeof(K), &key);
    gas_get_attribute(this, index, &retval, 0, sizeof(V));
}

inline void Chunk::set_payload (GASunum size, const GASvoid *payload)
{
    gas_set_payload(this, size, payload);
}

#undef copy_to_field

// vim: sw=4 fdm=marker
