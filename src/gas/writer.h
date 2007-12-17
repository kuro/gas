
/**
 * @file writer.h
 * @brief writer definition
 */

#ifndef WRITER_H
#define WRITER_H

#include <gas/context.h>

typedef struct _gas_writer gas_writer;
struct _gas_writer
{
    gas_context* context;
    void *handle;
};

void gas_write_encoded_num_writer (gas_writer *writer, GASunum value);
void gas_write_writer (gas_writer *writer, chunk* self);

#endif /* WRITER_H defined */

/* vim: set sw=4 fdm=marker :*/
