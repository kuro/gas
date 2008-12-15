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
 * @file qiodevice-context.cpp
 * @brief Provides a bridge between gas and QIODevice.
 */

#include "context.h"
#include <QIODevice>

/**
 * @brief does nothing
 *
 * @warning
 * This callback does nothing.  It is assumed that the application opens the
 * device manually.
 */
GASresult gas_qiodevice_open (const char *name, const char *mode,
                              void **handle, void **user_data)
{
    return GAS_OK;
}

/**
 * @brief does nothing
 *
 * @warning
 * This callback does nothing.  It is assumed that the application closes the
 * device manually.
 */
GASresult gas_qiodevice_close (void *handle, void *user_data)
{
    return GAS_OK;
}

/**
 * @brief called by gas to read from the device.
 */
GASresult gas_qiodevice_read (void *handle, void *buffer,
                              unsigned int size_bytes, unsigned int *bytes_read,
                              void *user_data)
{
    QIODevice& io = *static_cast<QIODevice*>(handle);

    if (io.bytesAvailable() < size_bytes) {
        return GAS_ERR_UNKNOWN;
    }

    *bytes_read = io.read((char*)buffer, size_bytes);

    if (*bytes_read < 0) {
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}

/**
 * @brief called by gas to write to the device.
 */
GASresult gas_qiodevice_write (void *handle, void *buffer,
                               unsigned int size_bytes,
                               unsigned int *bytes_written, void *user_data)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    QIODevice& io = *static_cast<QIODevice*>(handle);

    //io.waitForBytesWritten();
    *bytes_written = io.write((char*)buffer, size_bytes);

    if (*bytes_written < 0) {
        return GAS_ERR_UNKNOWN;
    }

    return GAS_OK;
}

/**
 * @brief called by gas to write seek through the device.
 *
 * @warning only support SEEK_CUR.
 */
GASresult gas_qiodevice_seek (void *handle, unsigned long pos,
                              int whence, void *user_data)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    if (whence != SEEK_CUR) {
        return GAS_ERR_INVALID_PARAM;
    }

    QIODevice& io = *static_cast<QIODevice*>(handle);

    if (io.isSequential()) {
        if (static_cast<unsigned long>(io.bytesAvailable()) < pos) {
            /// @todo find a better error code
            return GAS_ERR_UNKNOWN;
        }
        // ignoring the QByteArray, other than to check its size
        if (static_cast<unsigned long>(io.read(pos).size()) != pos) {
            return GAS_ERR_UNKNOWN;
        }
    } else {
        if (! io.seek(io.pos() + pos)) {
            return GAS_ERR_UNKNOWN;
        }
    }

    return GAS_OK;
}

/**
 * @brief Creates a new gas context taylored for QIODevice.
 */
GAScontext* gas_new_qiodevice_context (void)
{
    GAScontext *ctx = gas_context_new();
    ctx->open = gas_qiodevice_open;
    ctx->close = gas_qiodevice_close;
    ctx->read = gas_qiodevice_read;
    ctx->write = gas_qiodevice_write;
    ctx->seek = gas_qiodevice_seek;
    return ctx;
}

// vim: sw=4 fdm=marker
