#ifndef SEARPC_CLIENT_H
#define SEARPC_CLIENT_H

#ifdef LIBSEARPC_EXPORTS
#define LIBSEARPC_API __declspec(dllexport)
#else
#define LIBSEARPC_API
#endif

#include <glib.h>
#include <glib-object.h>
#include <jansson.h>

#ifndef DFT_DOMAIN
#define DFT_DOMAIN g_quark_from_string(G_LOG_DOMAIN)
#endif

G_BEGIN_DECLS

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

struct _SearpcClient {
    TransportCB send;
    void *arg;
    
    AsyncTransportSend async_send;
    void *async_arg;
};

typedef struct _SearpcClient LIBSEARPC_API SearpcClient;

LIBSEARPC_API
SearpcClient *searpc_client_new ();

LIBSEARPC_API void
searpc_client_free (SearpcClient *client);

LIBSEARPC_API void
searpc_client_call (SearpcClient *client, const char *fname,
                    const char *ret_type, GType gobject_type,
                    void *ret_ptr, GError **error,
                    int n_params, ...);

LIBSEARPC_API int
searpc_client_call__int (SearpcClient *client, const char *fname,
                         GError **error, int n_params, ...);

LIBSEARPC_API gint64
searpc_client_call__int64 (SearpcClient *client, const char *fname,
                           GError **error, int n_params, ...);

LIBSEARPC_API char *
searpc_client_call__string (SearpcClient *client, const char *fname,
                            GError **error, int n_params, ...);

LIBSEARPC_API GObject *
searpc_client_call__object (SearpcClient *client, const char *fname,
                            GType object_type,
                            GError **error, int n_params, ...);

LIBSEARPC_API GList*
searpc_client_call__objlist (SearpcClient *client, const char *fname,
                             GType object_type,
                             GError **error, int n_params, ...);

LIBSEARPC_API json_t *
searpc_client_call__json (SearpcClient *client, const char *fname,
                          GError **error, int n_params, ...);


LIBSEARPC_API char*
searpc_client_transport_send (SearpcClient *client,
                              const gchar *fcall_str,
                              size_t fcall_len,
                              size_t *ret_len);



LIBSEARPC_API int
searpc_client_async_call__int (SearpcClient *client,
                               const char *fname,
                               AsyncCallback callback, void *cbdata,
                               int n_params, ...);

LIBSEARPC_API int
searpc_client_async_call__int64 (SearpcClient *client,
                                 const char *fname,
                                 AsyncCallback callback, void *cbdata,
                                 int n_params, ...);

LIBSEARPC_API int
searpc_client_async_call__string (SearpcClient *client,
                                  const char *fname,
                                  AsyncCallback callback, void *cbdata,
                                  int n_params, ...);

LIBSEARPC_API int
searpc_client_async_call__object (SearpcClient *client,
                                  const char *fname,
                                  AsyncCallback callback, 
                                  GType object_type, void *cbdata,
                                  int n_params, ...);

LIBSEARPC_API int
searpc_client_async_call__objlist (SearpcClient *client,
                                   const char *fname,
                                   AsyncCallback callback, 
                                   GType object_type, void *cbdata,
                                   int n_params, ...);

LIBSEARPC_API int
searpc_client_async_call__json (SearpcClient *client,
                                const char *fname,
                                AsyncCallback callback, void *cbdata,
                                int n_params, ...);


/* called by the transport layer, the rpc layer should be able to
 * modify the str, but not take ownership of it */
LIBSEARPC_API int
searpc_client_generic_callback (char *retstr, size_t len,
                                void *vdata, const char *errstr);


/* in case of transport error, the following code and message will be
 * set in GError */
#define TRANSPORT_ERROR  "Transport Error"
#define TRANSPORT_ERROR_CODE 500

G_END_DECLS

#endif
