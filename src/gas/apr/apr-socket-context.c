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
 * @file apr-socket-context.cpp
 * @brief Provides a bridge between gas and apr.
 */

#include "context.h"
#include <apr_network_io.h>

#if GAS_DEBUG && HAVE_FPRINTF
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
GASresult gas_apr_socket_open (const char *name, const char *mode,
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
GASresult gas_apr_socket_close (void *handle, void *user_data)
{/*{{{*/
    return GAS_OK;
}/*}}}*/

/**
 * @brief called by gas to read from the device.
 */
static
GASresult gas_apr_socket_read (void *handle, void *buffer,
                               unsigned int size_bytes,unsigned int *bytes_read,
                               void *user_data)
{/*{{{*/
    apr_socket_t* socket = (apr_socket_t*)handle;
    apr_status_t status;
    apr_size_t len;

    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    len = size_bytes;
    status = apr_socket_recv(socket, buffer, &len);
    *bytes_read = len;

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
GASresult gas_apr_socket_write (void *handle, void *buffer,
                               unsigned int size_bytes,
                               unsigned int *bytes_written, void *user_data)
{/*{{{*/
    apr_socket_t* socket = (apr_socket_t*)handle;
    apr_status_t status;
    apr_size_t len;

    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    len = size_bytes;
    status = apr_socket_send(socket, buffer, &len);
    *bytes_written = len;

    if (status != APR_SUCCESS) {
        print_apr_error(status);
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}/*}}}*/

/**
 * @brief called by gas to write seek through the device.
 *
 * @warning only support GAS_SEEK_CUR.
 */
static
GASresult gas_apr_socket_seek (void *handle, unsigned long pos,
                               int whence, void *user_data)
{/*{{{*/
    apr_socket_t* socket = (apr_socket_t*)handle;
    apr_status_t status;
    char buf[1024];
    apr_size_t bytes_remaining, len;

    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    if (whence != GAS_SEEK_CUR) {
        return GAS_ERR_INVALID_PARAM;
    }

    bytes_remaining = pos;
    while (bytes_remaining > 0) {
        len = bytes_remaining > sizeof(buf) ? sizeof(buf) : bytes_remaining;
        status = apr_socket_recv(socket, buf, &len);
        if (status != APR_SUCCESS) {
            print_apr_error(status);
            return GAS_ERR_UNKNOWN;
        }
        bytes_remaining -= len;
    }

    return GAS_OK;
}/*}}}*/

/**
 * @brief Creates a new gas context taylored for apr files.
 */
GAScontext* gas_new_apr_socket_context (GASvoid* user_data)
{/*{{{*/
    GAScontext *ctx = NULL;

    gas_context_new(&ctx, user_data);

    ctx->open = gas_apr_socket_open;
    ctx->close = gas_apr_socket_close;
    ctx->read = gas_apr_socket_read;
    ctx->write = gas_apr_socket_write;
    ctx->seek = gas_apr_socket_seek;
    return ctx;
}/*}}}*/

// vim: sw=4 fdm=marker
