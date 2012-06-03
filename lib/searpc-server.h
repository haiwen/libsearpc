#ifndef SEARPC_SERVER_H
#define SEARPC_SERVER_H

#include <glib.h>

#ifndef DFT_DOMAIN
#define DFT_DOMAIN g_quark_from_string(G_LOG_DOMAIN)
#endif

struct _JsonArray;
typedef gchar* (*SearpcMarshalFunc) (void *func, struct _JsonArray *param_array,
    gsize *ret_len);

/**
 * searpc_server_init:
 *
 * Inititalize searpc server.
 */
void searpc_server_init ();

/**
 * searpc_server_final:
 * 
 * Free the server structure.
 */
void searpc_server_final ();

/**
 * searpc_create_service:
 *
 * Create a new service. Service is a set of functions.
 * The new service will be registered to the server.
 *
 * @svc_name: Service name.
 */
int searpc_create_service (const char *svc_name);

/**
 * searpc_remove_service:
 *
 * Remove the service from the server.
 */
void searpc_remove_service (const char *svc_name);

/**
 * searpc_server_register_marshal:
 *
 * For user to extend marshal functions.
 *
 * @signature: the signature of the marshal, register_marshal() will take
 * owner of this string.
 */
gboolean searpc_server_register_marshal (gchar *signature,
                                         SearpcMarshalFunc marshal);

/**
 * searpc_server_register_function:
 *
 * Register a rpc function with given signature to a service.
 *
 * @signature: the signature of the function, register_function() will take
 * owner of this string.
 */
gboolean searpc_server_register_function (const char *service,
                                          void* func,
                                          const gchar *fname,
                                          gchar *signature);

/**
 * searpc_server_call_function:
 * @service: service name.
 * @func: the serialized representation of the function to call.
 * @len: length of @func.
 * @ret_len: the length of the returned string.
 *
 * Call a registered function @func of a service.
 *
 * Returns the serialized representatio of the returned value.
 */
gchar *searpc_server_call_function (const char *service,
                                    gchar *func, gsize len, gsize *ret_len);

/**
 * searpc_compute_signature:
 * @ret_type: the return type of the function.
 * @pnum: number of parameters of the function.
 *
 * Compute function signature.
 */
char* searpc_compute_signature (gchar *ret_type, int pnum, ...);

/* Signatures */
#include <searpc-signature.h>

#endif
