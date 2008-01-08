
/**
 * @file test.cpp
 * @brief test implementation
 */

#include <gas/parser.h>
#include <gas/ntstring.h>

#include <stdio.h>
#include <string.h>

int total_bytes_read;

/* custom context {{{*/
GASnum my_open (const char *name, const char *mode, void **handle, void **userdata)
{
    *userdata = NULL;

    if (strcmp(name, "-") == 0) {
        if (mode[0] == 'w') {
            *handle = stdout;
            return GAS_OK;
        } else if (mode[0] == 'r') {
            *handle = stdin;
            return GAS_OK;
        }
    }

    FILE *fp;
    fp = fopen(name, mode);
    if (!fp) {
        return GAS_ERR_FILE_NOT_FOUND;
    }

    *handle = fp;

    return GAS_OK;
}

GASnum my_close (void *handle, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    fclose((FILE *)handle);

    return GAS_OK;
}

GASnum my_read (void *handle, void *buffer, unsigned int sizebytes,
                         unsigned int *bytesread, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    if (sizebytes == 0) {
        return GAS_OK;
    }

    if (bytesread) {
        *bytesread = (int)fread(buffer, 1, sizebytes, (FILE *)handle);

        total_bytes_read += *bytesread;

        if (*bytesread == 0) {
            printf("eof: total=%d\n", total_bytes_read);
            printf("requested %d\n", sizebytes);
        }

        if (*bytesread < sizebytes) {
            return GAS_ERR_FILE_EOF;
        }
    }

    return GAS_OK;
}

GASnum my_write (void *handle, void *buffer, unsigned int sizebytes,
                          unsigned int *byteswritten, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    if (byteswritten) {
        *byteswritten = (int)fwrite(buffer, 1, sizebytes, (FILE *)handle);

        if (*byteswritten < sizebytes) {
            return GAS_ERR_UNKNOWN;
        }
    }

    return GAS_OK;
}

GASnum my_seek (void *handle, unsigned int pos, int whence, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    /*fseek((FILE *)handle, pos, SEEK_SET);*/
    /** @todo make this configurable */
    fseek((FILE *)handle, pos, whence);

    return GAS_OK;
}

/*}}}*/

static int indent_level = -1;

void indent (void)
{
    int i;
    for (i = 0; i < indent_level; i++) {
        printf("  ");
    }
}

/* my callbacks {{{*/
GASbool my_pre_chunk (GASunum id_size, void *id, void *user_data)
{
    return GAS_TRUE;
}

void my_push_id (GASunum id_size, void *id, void *user_data)
{
    indent_level++;
    indent();  printf("---\n");
    indent();  printf("chunk: \"%s\"\n", (char*)id);
}

//void my_push_chunk (chunk* c, void *user_data)
//{
//    /*gas_print(c);*/
//}

void my_pop_id (GASunum id_size, void *id, void *user_data)
{
    indent_level--;
}

//void my_pop_chunk (chunk* c, void *user_data)
//{
//}

void my_on_attribute (GASunum key_size, void *key,
                   GASunum value_size, void *value,
                   void *user_data)
{
    return;
    indent();  printf("attribute: \"%s\" -> \"%s\"\n", (char*)key, (char*)value);
}

void my_on_payload (GASunum payload_size, void *payload, void *user_data)
{
    indent();  printf("payload: \"%s\"\n", (char*)payload);
}
/*}}}*/


int test_main (int argc, char **argv)
{
    chunk *c;
    gas_context *ctx;
    gas_parser *p;

    ctx = gas_context_new();
    ctx->open = my_open;
    ctx->close = my_close;
    ctx->read = my_read;
    ctx->write = my_write;
    ctx->seek = my_seek;

    p = gas_parser_new(ctx);

    p->build_tree = GAS_FALSE;
    //p->get_payloads = GAS_FALSE;

    p->on_pre_chunk = my_pre_chunk;

    p->on_push_id = my_push_id;
    p->on_payload = my_on_payload;
    p->on_attribute = my_on_attribute;
    p->on_pop_id = my_pop_id;

    total_bytes_read = 0;
    //c = gas_parse(p, "dump.gas");
    //c = gas_parse(p, "xslspec.gas");
    printf("%d\n", argc);
    if (argc == 1) {
        c = gas_parse(p, "-");
    } else if (argc == 2) {
        c = gas_parse(p, argv[1]);
    } else {
        fprintf(stderr, "test: invalid usage\n");
    }

    gas_parser_destroy(p);
    //gas_print(c);
    gas_destroy(c);
    gas_context_destroy(ctx);

    return 0;
}

// vim: sw=4 fdm=marker
