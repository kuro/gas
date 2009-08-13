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

#include <string.h>
#include <iostream>

#include <gas/ntstring.h>

#ifndef GAS_TREE_INL
#define GAS_TREE_INL

template <typename T>
inline void gas_hexdump (T x)
{
    gas_hexdump(x, sizeof(x));
}


namespace Gas
{

/* GAS::Exception methods {{{*/
inline Exception::Exception (const GASchar message[]) throw()
{
    memset(this->message, 0, sizeof(this->message));
    strncpy(this->message, message, sizeof(this->message) - 1);
}
inline char* Exception::what () throw()
{
    return (char*)message;
}
/*}}}*/

#define GAS_CHECK_RESULT(r)                                                 \
    if (r < 0) { throw Gas::Exception(gas_error_string(r)); }

/* macro copy_to_field() {{{*/
#define copy_to_field(field)                                                \
    do {                                                                    \
        this->field##_size = field##_size;                                  \
        this->field = (GASubyte*)gas_realloc(this->field, field##_size +1,  \
                                             user_data); \
        if (this->field == NULL) { throw Gas::Exception("out of memory"); } \
        memcpy(this->field, field, field##_size);                           \
        ((GASubyte*)this->field)[field##_size] = 0;                         \
    } while (0)
/*}}}*/

inline Chunk::Chunk (GASunum id_size, const GASvoid *id, GASvoid* user_data) :/*{{{*/
    parent(0),
    size(0),
    id_size(0),
    id(0),
    nb_attributes(0),
    attributes(0),
    payload_size(0),
    payload(0),
    nb_children(0),
    children(0)
{
    if (id) {
        copy_to_field(id);
    }
}/*}}}*/
inline Chunk::Chunk (const GASchar *id, GASvoid* user_data) :/*{{{*/
    parent(0),
    size(0),
    id_size(0),
    id(0),
    nb_attributes(0),
    attributes(0),
    payload_size(0),
    payload(0),
    nb_children(0),
    children(0)
{
    if (id) {
        id_size = strlen(id);
        copy_to_field(id);
    }
}/*}}}*/
inline Chunk::~Chunk ()/*{{{*/
{
    GASunum i;

    gas_free(id, this->user_data);
    for (i = 0; i < nb_attributes; i++) {
        gas_free(attributes[i].key, this->user_data);
        gas_free(attributes[i].value, this->user_data);
    }
    gas_free(attributes, this->user_data);
    gas_free(payload, this->user_data);
    for (i = 0; i < nb_children; i++) {
        delete children[i];
    }
    gas_free(children, this->user_data);
}/*}}}*/

/**
 * @param auto_swap when true, calls htons() or htonl()
 */
template<typename V>
inline GASvoid Chunk::set_attribute (const GASchar* key, const V& val, bool auto_swap)/*{{{*/
{
    if (!auto_swap) {
        gas_set_attribute(this, key, strlen(key), &val, sizeof(V));
    } else {
        V tmp;
        switch (sizeof(V)) {
        case 1:
            tmp = val;
            break;
        case 2:
            tmp = htons(val);
            break;
        case 4:
            tmp = htonl(val);
            break;
        default:
            throw Gas::Exception("unsupported type size for swap");
            break;
        }
        gas_set_attribute(this, key, strlen(key), &tmp, sizeof(V));
    }
}/*}}}*/

template<typename V>
inline GASvoid Chunk::set_attribute (GASchar* key, const V& val)/*{{{*/
{
    gas_set_attribute(this, key, strlen(key), &val, sizeof(V));
}/*}}}*/

inline GASvoid Chunk::set_attribute (const GASchar* key, const GASchar* val)/*{{{*/
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}/*}}}*/

inline GASvoid Chunk::set_attribute (GASchar* key, const GASchar* val)/*{{{*/
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}/*}}}*/

inline GASvoid Chunk::set_attribute (const GASchar* key, GASchar* val)/*{{{*/
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}/*}}}*/

inline GASvoid Chunk::set_attribute (GASchar* key, GASchar* val)/*{{{*/
{
    gas_set_attribute(this, key, strlen(key), val, strlen(val));
}/*}}}*/

template<typename K, typename V>
inline GASvoid Chunk::set_attribute (const K& key, const V& val)/*{{{*/
{
    gas_set_attribute(this, &key, sizeof(K), &val, sizeof(V));
}/*}}}*/

inline GASchar* Chunk::get_attribute (const GASchar* key)
{
    return gas_get_attribute_ss(this, key);
}

template<typename V>
inline GASvoid Chunk::get_attribute (const GASchar* key, V& retval, bool auto_swap) /*{{{*/
{
    GASresult r;
    GASunum len;
#if GAS_DEBUG
    GASnum index;
    index = gas_index_of_attribute(this, key, strlen(key));
    GAS_CHECK_RESULT(index);
    if (sizeof(retval) != this->attributes[index].value_size) {
#if HAVE_FPRINTF
        fprintf(stderr, "gas warning: value size mismatch\n");
#endif
    }
#endif
    len = sizeof(retval);
    r = gas_get_attribute_s(this, key, &retval, &len);
    GAS_CHECK_RESULT(r);
    if (auto_swap) {
        switch (sizeof(V)) {
        case 1:
            break;
        case 2:
            retval = ntohs(retval);
            break;
        case 4:
            retval = ntohl(retval);
            break;
        default:
            throw Gas::Exception("unsupported type size for swap");
            break;
        }
    }
}/*}}}*/

template<typename K, typename V>
inline GASvoid Chunk::get_attribute (const K& key, V& retval)/*{{{*/
{
    GASnum r;

    r = gas_index_of_attribute(this, &key, sizeof(K));
    GAS_CHECK_RESULT(r);

    r = gas_get_attribute(this, r, &retval, sizeof(V));
    GAS_CHECK_RESULT(r);
}/*}}}*/

inline GASvoid Chunk::set_payload (const GASvoid *payload, GASunum size)/*{{{*/
{
    gas_set_payload(this, payload, size);
}/*}}}*/
inline GASvoid Chunk::set_payload (const GASchar *payload)/*{{{*/
{
    gas_set_payload_s(this, payload);
}/*}}}*/

inline Chunk* Chunk::add_child (Chunk* child)/*{{{*/
{
    gas_add_child(this, child);
    return this;
}/*}}}*/

inline Chunk* Chunk::update (void)/*{{{*/
{
    gas_update(this);
    return this;
}/*}}}*/

inline Chunk* Chunk::operator<< (Chunk* child)/*{{{*/
{
    gas_add_child(this, child);
    return this;
}/*}}}*/

inline GASbool Chunk::has_attribute (const GASchar* key)
{
    return gas_has_attribute(this, key, strlen(key));
}

inline GASunum Chunk::totalSize () const
{
    return gas_total_size(const_cast<Chunk*>(this));
}

} // naemspace Gas

#undef GAS_CHECK_RESULT
#undef copy_to_field

#endif // GAS_TREE_INL defined

// vim: sw=4 fdm=marker
