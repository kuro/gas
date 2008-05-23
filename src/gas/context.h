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
 * @file context.h
 * @brief context definition
 */

#include <gas/types.h>

#ifndef GAS_SESSION_H
#define GAS_SESSION_H

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @name fd io */
/*@{*/

typedef GASresult (*GAS_FILE_OPEN_CALLBACK)  (const char *name, const char *mode,
                                              void **handle, void **userdata);
typedef GASresult (*GAS_FILE_CLOSE_CALLBACK) (void *handle, void *userdata);
typedef GASresult (*GAS_FILE_READ_CALLBACK)  (void *handle, void *buffer,
                                              unsigned int sizebytes,
                                              unsigned int *bytesread,
                                              void *userdata);
typedef GASresult (*GAS_FILE_WRITE_CALLBACK) (void *handle, void *buffer,
                                              unsigned int sizebytes,
                                              unsigned int *byteswritten,
                                              void *userdata);
/**
 * @brief provides the ability to seek forward (SEEK_CUR).
 */
typedef GASresult (*GAS_FILE_SEEK_CALLBACK)  (void *handle,
                                              unsigned long pos,
                                              int whence,
                                              void *userdata);

typedef struct
{
    GAS_FILE_OPEN_CALLBACK  open;
    GAS_FILE_CLOSE_CALLBACK close;
    GAS_FILE_READ_CALLBACK  read;
    GAS_FILE_WRITE_CALLBACK write;
    GAS_FILE_SEEK_CALLBACK  seek;
    void *user_data;
} GAScontext;

GAScontext* gas_context_new (void);
void gas_context_destroy (GAScontext* s);


/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_SESSION_H defined */

/* vim: set sw=4 fdm=marker : */
