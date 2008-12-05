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
 * @file context.c
 * @brief <++>
 */

#include "context.h"

#include <stdio.h>

GASresult gas_default_open (const char *name, const char *mode,
                            void **handle, void **userdata);
GASresult gas_default_close (void *handle, void *userdata);
GASresult gas_default_read (void *handle, void *buffer, unsigned int sizebytes,
                            unsigned int *bytesread, void *userdata);
GASresult gas_default_read (void *handle, void *buffer, unsigned int sizebytes,
                            unsigned int *bytesread, void *userdata);
GASresult gas_default_write (void *handle, void *buffer, unsigned int sizebytes,
                             unsigned int *byteswritten, void *userdata);
GASresult gas_default_seek (void *handle, unsigned long pos,
                            int whence, void *userdata);

#define USE_FILE 1
/* default callbacks {{{*/
GASresult gas_default_open (const char *name, const char *mode, void **handle, void **userdata)
{
#if USE_FILE
    FILE *fp;

    fp = fopen(name, mode);
    if (!fp) {
        return GAS_ERR_FILE_NOT_FOUND;
    }

    *userdata = NULL;
    *handle = (void*)fp;
#else
    long fd = 0;
    int flags = O_CREAT;
    if (mode == "r") {
        flags |= O_RDONLY;
    } else if (mode == "w") {
        flags |= O_WRONLY;
    } else if (mode == "rw") {
        flags |= O_RDWR;
    }

    fd = open(name, flags, 0644);
    if (fd == -1) {
        perror("error: File::File(filename, mode)");
        return GAS_ERR_FILE_NOT_FOUND;
    }

    *userdata = NULL;
    *handle = (void*)(long)fd;
#endif

    return GAS_OK;
}

GASresult gas_default_close (void *handle, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

#if USE_FILE
    fclose((FILE *)handle);
#else
    close((long)handle);
#endif

    return GAS_OK;
}

GASresult gas_default_read (void *handle, void *buffer, unsigned int sizebytes,
                         unsigned int *bytesread, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

#if USE_FILE
    if (bytesread) {
        *bytesread = (int)fread(buffer, 1, sizebytes, (FILE *)handle);

        if (*bytesread < sizebytes) {
            return GAS_ERR_FILE_EOF;
        }
    }
#else
    *bytesread = read((long)handle, buffer, sizebytes);
#endif

    return GAS_OK;
}

GASresult gas_default_write (void *handle, void *buffer, unsigned int sizebytes,
                          unsigned int *byteswritten, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

#if USE_FILE
    if (byteswritten) {
        *byteswritten = (int)fwrite(buffer, 1, sizebytes, (FILE *)handle);

        if (*byteswritten < sizebytes) {
            return GAS_ERR_UNKNOWN;
        }
    }
#else
    *byteswritten = write((long)handle, buffer, sizebytes);
#endif

    return GAS_OK;
}

GASresult gas_default_seek (void *handle, unsigned long pos,
                         int whence, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

#if USE_FILE
    fseek((FILE *)handle, pos, whence);
#else
    lseek((long)handle, pos, whence);
#endif

    return GAS_OK;
}
/*}}}*/

GAScontext* gas_context_new (void)
{
    GAScontext *s;
    s = (GAScontext*)gas_alloc(sizeof(GAScontext));
    if (s == NULL) { return NULL; }
    s->open = gas_default_open;
    s->close = gas_default_close;
    s->read = gas_default_read;
    s->write = gas_default_write;
    s->seek = gas_default_seek;
    s->user_data = NULL;
    return s;
}

void gas_context_destroy (GAScontext* s)
{
    gas_free(s);
}

/* vim: set sw=4 fdm=marker :*/
