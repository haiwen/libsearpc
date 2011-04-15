#ifndef SEARPC_SERVER_H
#define SEARPC_SERVER_H

#include <glib.h>


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
 * searpc_server_register_marshal:
 *
 * For user to extend marshal functions.
 */
gboolean searpc_server_register_marshal (const gchar *signature,
                                         SearpcMarshalFunc marshal);

/**
 * searpc_server_register_function:
 *
 * Register a rpc function with given signature.
 */
gboolean searpc_server_register_function (void* func,
                                          const gchar *fname,
                                          const gchar *signature);

/**
 * searpc_server_call_function:
 * @func: the serialized representation of the function to call.
 * @len: length of @func.
 * @ret_len: the length of the returned string.
 *
 * Call a registered function @func.
 *
 * Returns the serialized representatio of the returned value.
 */
gchar *searpc_server_call_function (gchar *func, gsize len, gsize *ret_len);

/**
 * searpc_compute_signature:
 * @ret_type: the return type of the function.
 * @pnum: number of parameters of the function.
 *
 * Compute function signature.
 */
char* searpc_compute_signature (gchar *ret_type, int pnum, ...);

/* Signatures */
#include "searpc-signature.h"

#endif
