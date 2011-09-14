#ifndef SEARPC_CLIENT_H
#define SEARPC_CLIENT_H

#include <glib.h>
#include <glib-object.h>

typedef char *(*TransportCB)(void *arg, const gchar *fcall_str,
                             size_t fcall_len, size_t *ret_len);

/**
 * @rpc_priv is used by the rpc_client to store information related to
 * this rpc call.
 * @fcall_str is an allocated string, and the sender should free it
 * when not needed.
 */
typedef int (*AsyncTransportSend)(void *arg, gchar *fcall_str,
                                  size_t fcall_len, void *rpc_priv);

typedef void (*AsyncCallback) (void *result, void *user_data, GError *error);

typedef struct {
    TransportCB transport;
    void *arg;
    
    AsyncTransportSend async_send;
    void *async_arg;
} SearpcClient;

SearpcClient *searpc_client_new ();

void searpc_client_free (SearpcClient *client);

char* searpc_client_transport_send (SearpcClient *client,
                                    const gchar *fcall_str,
                                    size_t fcall_len,
                                    size_t *ret_len);


/**
 * Send the serialized function call to server.
 *
 * @fcall_str: the serialized function.
 * @ret_type: the return type.
 * @gtype: specify the type id if @ret_type is `object` or `objlist`,
 *         or 0 otherwise.
 * @cbdata: the data that will be given to the callback.
 */
int searpc_client_async_call (SearpcClient *client,
                              gchar *fcall_str,
                              size_t fcall_len,
                              AsyncCallback callback,
                              const gchar *ret_type,
                              int gtype,
                              void *cbdata);

/* called by the transport layer, the rpc layer should be able to
 * modify the str, but not take ownership of it */
int
searpc_client_generic_callback (char *retstr, size_t len,
                                void *vdata, const char *errstr);

#include <searpc-fcall.h>


char*
searpc_client_fret__string (char *data, size_t len, GError **error);

int
searpc_client_fret__int (char *data, size_t len, GError **error);

GObject*
searpc_client_fret__object (GType gtype, char *data,
                            size_t len, GError **error);

GList*
searpc_client_fret__objlist (GType gtype, char *data,
                             size_t len, GError **error);

/* in case of transport error, the following code and message will be
 * set in GError */
#define TRANSPORT_ERROR  "Transport Error"
#define TRANSPORT_ERROR_CODE 500

#include <searpc-dfun.h>


#endif
