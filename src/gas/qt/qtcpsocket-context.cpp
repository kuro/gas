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
 * @file qtcpsocket-context.cpp
 * @brief Provides a bridge between gas and QTcpSocket.
 */

#include "context.h"
#include <QTcpSocket>


/**
 * @brief does nothing
 *
 * @warning
 * This callback does nothing.  It is assumed that the application opens the
 * socket manually.
 */
static
GASresult gas_qtcpsocket_open (const char *name, const char *mode, void **handle, void **user_data)
{// {{{
    return GAS_OK;
}// }}}

/**
 * @brief does nothing
 *
 * @warning
 * This callback does nothing.  It is assumed that the application closes the
 * socket manually.
 */
static
GASresult gas_qtcpsocket_close (void *handle, void *user_data)
{// {{{
    return GAS_OK;
}// }}}

/**
 * @brief called by gas to read from the socket.
 */
static
GASresult gas_qtcpsocket_read (void *handle, void *buffer,
                               unsigned int size_bytes,
                               unsigned int *bytes_read, void *user_data)
{// {{{
    QTcpSocket& io = *static_cast<QTcpSocket*>(handle);

    for (int i = 0; io.bytesAvailable() < size_bytes && i < 5; i++) {
        if (! io.waitForReadyRead(300000)) {
            if (! io.isValid()) {
                return GAS_ERR_UNKNOWN;
            }
        }
    }

    if (io.bytesAvailable() < size_bytes) {
        return GAS_ERR_UNKNOWN;
    }

    *bytes_read = io.read((char*)buffer, size_bytes);

    if (*bytes_read < 0) {
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}// }}}

/**
 * @brief called by gas to write to the socket.
 */
static
GASresult gas_qtcpsocket_write (void *handle, void *buffer,
                                unsigned int size_bytes,
                                unsigned int *bytes_written, void *user_data)
{// {{{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    QTcpSocket& io = *static_cast<QTcpSocket*>(handle);

    //io.waitForBytesWritten();
    *bytes_written = io.write((char*)buffer, size_bytes);

    if (*bytes_written < 0) {
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}// }}}

/**
 * @brief called by gas to write to the socket.
 */
static
GASresult gas_qtcpsocket_seek (void *handle, unsigned long pos,
                         int whence, void *user_data)
{// {{{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    if (whence != GAS_SEEK_CUR) {
        return GAS_ERR_INVALID_PARAM;
    }

    QTcpSocket& io = *static_cast<QTcpSocket*>(handle);

    // sockets are sequential devices

    if (static_cast<unsigned long>(io.bytesAvailable()) < pos) {
        /// @todo find a better error code
        return GAS_ERR_UNKNOWN;
    }

    // ignoring the QByteArray, other than to check its size
    if (static_cast<unsigned long>(io.read(pos).size()) != pos) {
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}// }}}

GAScontext* gas_new_qtcpsocket_context (void)
{// {{{
    GAScontext *ctx = NULL;

    gas_context_new(&ctx);

    ctx->open = gas_qtcpsocket_open;
    ctx->close = gas_qtcpsocket_close;
    ctx->read = gas_qtcpsocket_read;
    ctx->write = gas_qtcpsocket_write;
    ctx->seek = gas_qtcpsocket_seek;
    return ctx;
}// }}}

// vim: sw=4 fdm=marker
