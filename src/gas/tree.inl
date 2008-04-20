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
inline Chunk::Chunk (const GASchar *id)
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

inline GASvoid Chunk::add_child (Chunk* child)
{
    gas_add_child(this, child);
}

template<typename V>
inline GASvoid Chunk::set_attribute (const GASchar* key, const V& val)
{
    gas_set_attribute(this, key, strlen(key), &val, sizeof(V));
}

template<typename V>
inline GASvoid Chunk::set_attribute (GASchar* key, const V& val)
{
    gas_set_attribute(this, key, strlen(key), &val, sizeof(V));
}

inline GASvoid Chunk::set_attribute (const GASchar* key, const GASchar* val)
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}

inline GASvoid Chunk::set_attribute (GASchar* key, const GASchar* val)
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}

inline GASvoid Chunk::set_attribute (const GASchar* key, GASchar* val)
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}

inline GASvoid Chunk::set_attribute (GASchar* key, GASchar* val)
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}

template<typename K, typename V>
inline GASvoid Chunk::set_attribute (const K& key, const V& val)
{
    //std::clog << typeid(K).name() << std::endl;
    std::clog << typeid(key).name() << std::endl;
    std::clog << typeid(typeof(key)).name() << std::endl;
    std::clog << (typeid(key).name() == typeid(GASchar*).name()) << std::endl;
    std::clog << (1 xor 1) << std::endl;
    gas_set_attribute(this, &key, sizeof(K), &val, sizeof(V));
}

template<typename V>
inline GASvoid Chunk::get_attribute (const GASchar* key, V& retval)
{
    gas_get_attribute_s(this, key, &retval, sizeof(retval));
}

template<typename K, typename V>
inline GASvoid Chunk::get_attribute (const K& key, V& retval)
{
    puts(";)");
    GASnum index;
    index = gas_index_of_attribute(this, sizeof(K), &key);
    gas_get_attribute(this, index, &retval, sizeof(V));
}

inline GASvoid Chunk::set_payload (const GASvoid *payload, GASunum size)
{
    gas_set_payload(this, payload, size);
}

#undef copy_to_field

// vim: sw=4 fdm=marker
