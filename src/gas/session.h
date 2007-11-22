
/**
 * @file session.h
 * @brief session definition
 */

#ifndef GAS_SESSION_H
#define GAS_SESSION_H

#include <gas/gas.h>

#define GAS_OK 0
#define GAS_ERR_INVALID_PARAM  -1
#define GAS_ERR_FILE_NOT_FOUND -2
#define GAS_ERR_FILE_EOF -3
#define GAS_ERR_UNKNOWN -4

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
typedef GASnum (*GAS_FILE_SEEK_CALLBACK)  (void *handle,
                                           unsigned int pos,
                                           void *userdata);

typedef struct _gas_session gas_session;
struct _gas_session
{
    GAS_FILE_OPEN_CALLBACK  open_callback;
    GAS_FILE_CLOSE_CALLBACK close_callback;
    GAS_FILE_READ_CALLBACK  read_callback;
    GAS_FILE_WRITE_CALLBACK write_callback;
    GAS_FILE_SEEK_CALLBACK  seek_callback;
    void *user_data;
};

gas_session* gas_session_new (void);
void gas_session_destroy (gas_session* s);

void gas_write_cb (chunk* self, gas_session *session);
chunk* gas_read_fd (int fd);

void gas_write_encoded_num_fd (int fd, GASunum value);
GASunum gas_read_encoded_num_fd (int fd);

/*FMOD_RESULT F_API FMOD_System_SetFileSystem          (FMOD_SYSTEM *system, FMOD_FILE_OPENCALLBACK useropen, FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek, int blockalign);*/

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* GAS_SESSION_H defined */

/* vim: set sw=4 fdm=marker : */
