
/**
 * @file session.c
 * @brief <++>
 */

#include "session.h"

#include <stdio.h>



GASnum gas_default_open (const char *name, const char *mode, void **handle, void **userdata)
{
    if (name) {
        FILE *fp;

        fp = fopen(name, mode);
        if (!fp) {
            return GAS_ERR_FILE_NOT_FOUND;
        }

        /* fseek(fp, 0, SEEK_END); */
        /* *filesize = ftell(fp);  */
        /* fseek(fp, 0, SEEK_SET); */

        *userdata = (void *)0x12345678;
        *handle = fp;
    }

    return GAS_OK;
}

GASnum gas_default_close (void *handle, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    fclose((FILE *)handle);

    return GAS_OK;
}

GASnum gas_default_read (void *handle, void *buffer, unsigned int sizebytes,
                         unsigned int *bytesread, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    if (bytesread) {
        *bytesread = (int)fread(buffer, 1, sizebytes, (FILE *)handle);

        if (*bytesread < sizebytes) {
            return GAS_ERR_FILE_EOF;
        }
    }

    return GAS_OK;
}

GASnum gas_default_write (void *handle, void *buffer, unsigned int sizebytes,
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

GASnum gas_default_seek (void *handle, unsigned int pos, void *userdata)
{
    if (!handle) {
        return GAS_ERR_INVALID_PARAM;
    }

    /*fseek((FILE *)handle, pos, SEEK_SET);*/
    /** @todo make this configurable */
    fseek((FILE *)handle, pos, SEEK_CUR);

    return GAS_OK;
}

gas_session* gas_session_new (void)
{
    gas_session *s;
    s = malloc(sizeof(gas_session));
    s->open_callback = gas_default_open;
    s->close_callback = gas_default_close;
    s->read_callback = gas_default_read;
    s->write_callback = gas_default_write;
    s->seek_callback = gas_default_seek;
    s->user_data = NULL;
    return s;
}

void gas_session_destroy (gas_session* s)
{
    free(s);
}

/* vim: set sw=4 fdm=marker :*/
