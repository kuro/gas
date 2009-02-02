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
 * @file apr-file-context.cpp
 * @brief Provides a bridge between gas and apr.
 */

#include "context.h"
#include <apr_file_io.h>
#include <gas/tree.h>

#if GAS_DEBUG
#define print_apr_error(status)                                             \
    do {                                                                    \
        char err_buf[1024];                                                 \
        apr_strerror(status, err_buf, sizeof(err_buf));                     \
        fprintf(stderr, "[%s:%d] apr error: %" GAS_INT32_FMT ": %s\n",      \
                gas_basename(__FILE__), __LINE__,                           \
                status, err_buf);                                           \
    } while (0)
#else
#define print_apr_error(status)
#endif

/**
 * @brief does nothing
 *
 * @warning
 * This callback does nothing.  It is assumed that the application opens the
 * device manually.
 */
static
GASresult gas_apr_file_open (const char *name, const char *mode,
                             void **handle, void **user_data)
{/*{{{*/
    return GAS_OK;
}/*}}}*/

/**
 * @brief does nothing
 *
 * @warning
 * This callback does nothing.  It is assumed that the application closes the
 * device manually.
 */
static
GASresult gas_apr_file_close (void *handle, void *user_data)
{/*{{{*/
    return GAS_OK;
}/*}}}*/

/**
 * @brief called by gas to read from the device.
 */
static
GASresult gas_apr_file_read (void *handle, void *buffer,
                             unsigned int size_bytes, unsigned int *bytes_read,
                             void *user_data)
{/*{{{*/
    apr_file_t* file = (apr_file_t*)handle;
    apr_status_t status;
    apr_size_t br;

    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    status = apr_file_read_full(file, buffer, size_bytes, &br);
    *bytes_read = br;

    if (status != APR_SUCCESS) {
        if (APR_STATUS_IS_EOF(status)) {
            return GAS_ERR_FILE_EOF;
        } else {
            print_apr_error(status);
            return GAS_ERR_UNKNOWN;
        }
    }

    return GAS_OK;
}/*}}}*/

/**
 * @brief called by gas to write to the device.
 */
static
GASresult gas_apr_file_write (void *handle, void *buffer,
                               unsigned int size_bytes,
                               unsigned int *bytes_written, void *user_data)
{/*{{{*/
    apr_file_t* file = (apr_file_t*)handle;
    apr_status_t status;
    apr_size_t bw;

    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    status = apr_file_write_full(file, buffer, size_bytes, &bw);
    *bytes_written = bw;

    if (size_bytes == 0) {
        return GAS_OK;
    }

    if (status != APR_SUCCESS) {
        print_apr_error(status);
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}/*}}}*/

/**
 * @brief called by gas to write seek through the device.
 */
static
GASresult gas_apr_file_seek (void *handle, unsigned long pos,
                              int whence, void *user_data)
{/*{{{*/
    apr_file_t* file = (apr_file_t*)handle;
    apr_status_t status;
    apr_seek_where_t where;
    apr_off_t offset;

    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    switch(whence) {
    case SEEK_SET:
        where = APR_SET;
        break;
    case SEEK_CUR:
        where = APR_CUR;
        break;
    case SEEK_END:
        where = APR_END;
        break;
    default:
        return GAS_ERR_INVALID_PARAM;
    }

    offset = pos;
    status = apr_file_seek(file, where, &offset);

    if (status != APR_SUCCESS) {
        print_apr_error(status);
        return GAS_ERR_UNKNOWN;
    }

    if (offset != pos) {
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}/*}}}*/

/**
 * @brief Creates a new gas context taylored for apr files.
 */
GAScontext* gas_new_apr_file_context (GASvoid* user_data)
{/*{{{*/
    GAScontext *ctx = NULL;

    gas_context_new(&ctx, user_data);

    ctx->open = gas_apr_file_open;
    ctx->close = gas_apr_file_close;
    ctx->read = gas_apr_file_read;
    ctx->write = gas_apr_file_write;
    ctx->seek = gas_apr_file_seek;
    return ctx;
}/*}}}*/

// vim: sw=4 fdm=marker
