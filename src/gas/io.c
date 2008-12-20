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
 * @file io.c
 */

#include "io.h"
#include "parser.h"
#include "writer.h"

GASio* gas_io_new (GAScontext* context, GASvoid* user_data)
{
    GASio *io;

#ifdef GAS_DEBUG
    if (context == NULL) { return NULL; }
#endif

    io = (GASio*)gas_alloc(sizeof(GASio), user_data);
    if (io == NULL) { return NULL; }

    io->parser = gas_parser_new(context, user_data);
    io->writer = gas_writer_new(context, user_data);

    return io;
}
void gas_io_destroy (GASio *io, GASvoid* user_data)
{
    gas_parser_destroy(io->parser, user_data);
    gas_writer_destroy(io->writer, user_data);
    gas_free(io, user_data);
}

GASresult gas_io_set_handle (GASio* io, GASvoid* handle)
{
    GAS_CHECK_PARAM(io);

    io->parser->handle = handle;
    io->writer->handle = handle;

    return GAS_OK;
}

GASresult gas_read_io (GASio* io, GASchunk** out, GASvoid* user_data)
{
    return gas_read_parser(io->parser, out, user_data);
}
GASresult gas_write_io (GASio* io, GASchunk* c)
{
    return gas_write_writer(io->writer, c);
}

// vim: sw=4 fdm=marker
