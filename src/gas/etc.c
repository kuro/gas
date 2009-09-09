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
 * @file etc.c
 */

#include "tree.h"
#include <string.h>
#include <ctype.h>

/** @name helper functions */
/*@{*/

const char* gas_basename (const char* path)/*{{{*/
{
    const char* p;
    for (p = path + strlen(path) - 1; p >= path; p--)
    {
        if (*p == '/')
        {
            return p + 1;
        }
    }
    return path;
}/*}}}*/

GASchar* gas_error_string (GASresult result)/*{{{*/
{
    switch (result) {
    case GAS_OK:                  return "no error";
    case GAS_ERR_INVALID_PARAM:   return "invalid parameter";
    case GAS_ERR_FILE_NOT_FOUND:  return "file not found";
    case GAS_ERR_FILE_EOF:        return "end of file";
    case GAS_ERR_ATTR_NOT_FOUND:  return "attribute not found";
    case GAS_ERR_OUT_OF_RANGE:    return "value out of range";
    case GAS_ERR_MEMORY:          return "out of memory";
    case GAS_ERR_UNKNOWN:         return "unknown error";
    default: return (result > 0) ? "no error" : "invalid error code";
    }
}/*}}}*/

int gas_cmp (const GASubyte *a, GASunum a_len, const GASubyte *b, GASunum b_len)/*{{{*/
{
    int result;
    unsigned int i = 0;

    GAS_CHECK_PARAM(a);
    GAS_CHECK_PARAM(b);

    while (1) {
        if (i == a_len) {
            result = (a_len == b_len) ? 0 : -1;
            break;
        }
        if (i == b_len) {
            result = (a_len == b_len) ? 0 : 1;
            break;
        }
        if (a[i] < b[i]) {
            result = -1;
            break;
        }
        if (a[i] > b[i]) {
            result = 1;
            break;
        }
        i++;
    }
    return result;
}/*}}}*/

#if HAVE_FPRINTF
GASresult gas_hexdump_f (FILE* fs, const GASvoid *input, GASunum size)/*{{{*/
{
    GASunum i, x, o;
    GASubyte *buf = (GASubyte*)input;
    GASchar characters[17];

    if (size == 0) {
        fprintf(fs, "%07lx: \n", 0x0l);
        return GAS_OK;
    }

    GAS_CHECK_PARAM(fs);
    GAS_CHECK_PARAM(input);

    characters[16] = '\0';

    o = 0;
    for (i = 0; i < (size >> 4); i++) {
        fprintf(fs, "%07lx: ", i << 4);
        for (x = 0; x < 8; x++) {
            fprintf(fs, "%02x%02x ", buf[o], buf[o+1]);
            characters[x << 1] = isprint(buf[o]) ? buf[o] : '.';
            characters[( x << 1 ) + 1] = isprint(buf[o+1]) ? buf[o+1] : '.';
            o += 2;
        }
        fprintf(fs, " %s", characters);
        fprintf(fs, "\n");
    }

    memset(characters, 0, 16);

    /* finally */
    fprintf(fs, "%07lx: ", (size >> 4) << 4);
    x = 0;
    for (i = 0; i < (size % 16); i++) {
        fprintf(fs, "%02x", buf[o]);
        characters[i] = isprint(buf[o]) ? buf[o] : '.';
        if (x) {
            fprintf(fs, " ");
        }
        x = ! x;
        o += 1;
    }
    for (; i < 16; i++) {
        fprintf(fs, "  ");
        if (x) {
            fprintf(fs, " ");
        }
        x = ! x;
    }
    fprintf(fs, " %s\n", characters);

    return GAS_OK;
}/*}}}*/
GASresult gas_hexdump (const GASvoid *input, GASunum size)/*{{{*/
{
    return gas_hexdump_f(stderr, input, size);
}/*}}}*/
#endif

/**
 * @brief Checks to see if @a id matches id of @a c.
 *
 * @retval true match
 * @retval false no match
 */
GASbool gas_id_is (const GASchunk* c, const GASchar* id)/*{{{*/
{
    return gas_cmp(c->id, c->id_size, (GASubyte*)id, strlen(id)) == 0;
}/*}}}*/

/*@}*/

/* vim: set sw=4 fdm=marker : */
