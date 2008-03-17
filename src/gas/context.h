
/**
 * @file context.h
 * @brief context definition
 */

#ifndef GAS_SESSION_H
#define GAS_SESSION_H

#include <gas/types.h>

#ifdef __cplusplus
extern "C"
{
/*}*/
#endif


/** @name fd io */
/*@{*/

typedef GASnum (*GAS_FILE_OPEN_CALLBACK)  (const char *name, const char *mode,
                                           void **handle, void **userdata);
typedef GASnum (*GAS_FILE_CLOSE_CALLBACK) (void *handle, void *userdata);
typedef GASnum (*GAS_FILE_READ_CALLBACK)  (void *handle, void *buffer,
                                           unsigned int sizebytes,
                                           unsigned int *bytesread,
                                           void *userdata);
typedef GASnum (*GAS_FILE_WRITE_CALLBACK) (void *handle, void *buffer,
                                           unsigned int sizebytes,
                                           unsigned int *byteswritten,
                                           void *userdata);
/**
 * @brief provides the ability to seek forward (SEEK_CUR).
 */
typedef GASnum (*GAS_FILE_SEEK_CALLBACK)  (void *handle,
                                           unsigned long pos,
                                           int whence,
                                           void *userdata);

typedef struct _gas_context gas_context;

/**
 * @see _gas_context
 */
struct _gas_context
{
    GAS_FILE_OPEN_CALLBACK  open;
    GAS_FILE_CLOSE_CALLBACK close;
    GAS_FILE_READ_CALLBACK  read;
    GAS_FILE_WRITE_CALLBACK write;
    GAS_FILE_SEEK_CALLBACK  seek;
    void *user_data;
};

gas_context* gas_context_new (void);
void gas_context_destroy (gas_context* s);


/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_SESSION_H defined */

/* vim: set sw=4 fdm=marker : */
