#ifndef SEARPC_CLIENT_H
#define SEARPC_CLIENT_H

#include <glib.h>
#include <glib-object.h>

typedef char *(*TransportCB)(void *arg, const gchar *fcall_str,
                             size_t fcall_len, size_t *ret_len);

typedef struct {
    TransportCB transport;
    void *arg;
} SearpcClient;

SearpcClient *searpc_client_new ();
void searpc_client_free (SearpcClient *client);

char* searpc_client_transport_send (SearpcClient *client,
                                    const gchar *fcall_str,
                                    size_t fcall_len,
                                    size_t *ret_len);

#include <searpc-fcall.h>

/**
 * searpc_client_fret__string:
 * @data:  the return string of RPC call
 * @len: then length of @data
 * @error: return location for a #GError, or %NULL
 * 
 * Extract return value of type `char*` from the return string of RPC
 * call.
 */
char* searpc_client_fret__string (char *data, size_t len, GError **error);

int   searpc_client_fret__int (char *data, size_t len, GError **error);
GObject* searpc_client_fret__object (GType gtype, char *data,
                                     size_t len, GError **error);
GList* searpc_client_fret__objlist (GType gtype, char *data,
                                    size_t len, GError **error);


#define TRANSPORT_ERROR  "Transport Error"
#define TRANSPORT_ERROR_CODE 500

#include <searpc-dfun.h>


#endif
