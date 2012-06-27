#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json-glib/json-glib.h"

#include "searpc-client.h"
#include "searpc-utils.h"


static char*
searpc_client_fret__string (char *data, size_t len, GError **error);

static int
searpc_client_fret__int (char *data, size_t len, GError **error);

static gint64
searpc_client_fret__int64 (char *data, size_t len, GError **error);

static GObject*
searpc_client_fret__object (GType gtype, char *data,
                            size_t len, GError **error);

static GList*
searpc_client_fret__objlist (GType gtype, char *data,
                             size_t len, GError **error);


static void clean_objlist(GList *list)
{
    GList *ptr;
    for (ptr = list; ptr; ptr = ptr->next)
        g_object_unref(ptr->data);
    g_list_free (list);
}


SearpcClient *
searpc_client_new ()
{
    return g_new0 (SearpcClient, 1);
}

void
searpc_client_free (SearpcClient *client)
{
    if (!client)
        return;

    g_free (client);
}

char *
searpc_client_transport_send (SearpcClient *client,
                              const gchar *fcall_str,
                              size_t fcall_len,
                              size_t *ret_len)
{
    return client->send(client->arg, fcall_str,
                        fcall_len, ret_len);
}

static char *
fcall_to_str (const char *fname, int n_params, va_list args, gsize *len)
{
    JsonArray *array;
    
    array = json_array_new ();
    json_array_add_string_element (array, fname);

    int i = 0;
    for (; i < n_params; i++) {
        const char *type = va_arg(args, const char *);
        void *value = va_arg(args, void *);
        if (strcmp(type, "int") == 0)
            json_array_add_int_element (array, (int)value);
        else if (strcmp(type, "int64") == 0)
            json_array_add_int_element (array, *((gint64 *)value));
        else if (strcmp(type, "string") == 0)
            json_array_add_string_or_null_element (array, (char *)value);
        else {
            g_warning ("unrecognized parameter type %s\n", type);
            return NULL;
        }
    }

    gchar *data;
    JsonGenerator *gen = json_generator_new ();
    JsonNode *root;

    root = json_node_new (JSON_NODE_ARRAY);
    json_node_take_array (root, array);
    json_generator_set_root (gen, root);

    g_object_set (gen, "pretty", FALSE, NULL);
    data = json_generator_to_data (gen, len);

    json_node_free (root);
    g_object_unref (gen);

    return data;
}

void
searpc_client_call (SearpcClient *client, const char *fname,
                    const char *ret_type, GType gobject_type,
                    void *ret_ptr, GError **error,
                    int n_params, ...)
{
    g_return_if_fail (fname != NULL);
    g_return_if_fail (ret_type != NULL);

    va_list args;
    gsize len, ret_len;
    char *fstr;

    va_start (args, n_params);
    fstr = fcall_to_str (fname, n_params, args, &len);
    va_end (args);
    if (!fstr) {
        g_set_error (error, DFT_DOMAIN, 0, "Invalid Parameter");
        return;
    }
 
    char *fret = searpc_client_transport_send (client, fstr, len, &ret_len);
    if (!fret) {
        g_free (fstr);
        g_set_error (error, DFT_DOMAIN, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);
        return;
    }

    if (strcmp(ret_type, "int") == 0)
        *((int *)ret_ptr) = searpc_client_fret__int (fret, ret_len, error);
    else if (strcmp(ret_type, "int64") == 0)
        *((gint64 *)ret_ptr) = searpc_client_fret__int64 (fret, ret_len, error);
    else if (strcmp(ret_type, "string") == 0)
        *((char **)ret_ptr) = searpc_client_fret__string (fret, len, error);
    else if (strcmp(ret_type, "object") == 0)
        *((GObject **)ret_ptr) = searpc_client_fret__object (gobject_type, fret,
                                                             ret_len, error);
    else if (strcmp(ret_type, "objlist") == 0)
        *((GList **)ret_ptr) = searpc_client_fret__objlist (gobject_type, fret,
                                                            ret_len, error);
    else
        g_warning ("unrecognized return type %s\n", ret_type);
    
    g_free (fstr);
    g_free (fret);
}

int
searpc_client_call__int (SearpcClient *client, const char *fname,
                         GError **error, int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, 0);

    va_list args;
    gsize len, ret_len;
    char *fstr;

    va_start (args, n_params);
    fstr = fcall_to_str (fname, n_params, args, &len);
    va_end (args);
    if (!fstr) {
        g_set_error (error, DFT_DOMAIN, 0, "Invalid Parameter");
        return 0;
    }
 
    char *fret = searpc_client_transport_send (client, fstr, len, &ret_len);
    if (!fret) {
        g_free (fstr);
        g_set_error (error, DFT_DOMAIN, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);
        return 0;
    }

    int ret = searpc_client_fret__int (fret, ret_len, error);
    g_free (fstr);
    g_free (fret);
    return ret;
}

gint64
searpc_client_call__int64 (SearpcClient *client, const char *fname,
                         GError **error, int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, 0);

    va_list args;
    gsize len, ret_len;
    char *fstr;

    va_start (args, n_params);
    fstr = fcall_to_str (fname, n_params, args, &len);
    va_end (args);
    if (!fstr) {
        g_set_error (error, DFT_DOMAIN, 0, "Invalid Parameter");
        return 0;
    }
 
    char *fret = searpc_client_transport_send (client, fstr, len, &ret_len);
    if (!fret) {
        g_free (fstr);
        g_set_error (error, DFT_DOMAIN, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);
        return 0;
    }

    gint64 ret = searpc_client_fret__int64 (fret, ret_len, error);
    g_free (fstr);
    g_free (fret);
    return ret;
}

char *
searpc_client_call__string (SearpcClient *client, const char *fname,
                            GError **error, int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, NULL);

    va_list args;
    gsize len, ret_len;
    char *fstr;

    va_start (args, n_params);
    fstr = fcall_to_str (fname, n_params, args, &len);
    va_end (args);
    if (!fstr) {
        g_set_error (error, DFT_DOMAIN, 0, "Invalid Parameter");
        return NULL;
    }
 
    char *fret = searpc_client_transport_send (client, fstr, len, &ret_len);
    if (!fret) {
        g_free (fstr);
        g_set_error (error, DFT_DOMAIN, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);
        return NULL;
    }

    char *ret = searpc_client_fret__string (fret, ret_len, error);
    g_free (fstr);
    g_free (fret);
    return ret;
}

GObject *
searpc_client_call__object (SearpcClient *client, const char *fname,
                            GType object_type,
                            GError **error, int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, NULL);
    g_return_val_if_fail (object_type != 0, NULL);

    va_list args;
    gsize len, ret_len;
    char *fstr;

    va_start (args, n_params);
    fstr = fcall_to_str (fname, n_params, args, &len);
    va_end (args);
    if (!fstr) {
        g_set_error (error, DFT_DOMAIN, 0, "Invalid Parameter");
        return NULL;
    }
 
    char *fret = searpc_client_transport_send (client, fstr, len, &ret_len);
    if (!fret) {
        g_free (fstr);
        g_set_error (error, DFT_DOMAIN, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);
        return NULL;
    }

    GObject *ret = searpc_client_fret__object (object_type, fret, ret_len, error);
    g_free (fstr);
    g_free (fret);
    return ret;
}

GList*
searpc_client_call__objlist (SearpcClient *client, const char *fname,
                             GType object_type,
                             GError **error, int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, NULL);
    g_return_val_if_fail (object_type != 0, NULL);

    va_list args;
    gsize len, ret_len;
    char *fstr;

    va_start (args, n_params);
    fstr = fcall_to_str (fname, n_params, args, &len);
    va_end (args);
    if (!fstr) {
        g_set_error (error, DFT_DOMAIN, 0, "Invalid Parameter");
        return NULL;
    }
 
    char *fret = searpc_client_transport_send (client, fstr, len, &ret_len);
    if (!fret) {
        g_free (fstr);
        g_set_error (error, DFT_DOMAIN, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);
        return NULL;
    }

    GList *ret = searpc_client_fret__objlist (object_type, fret, ret_len, error);
    g_free (fstr);
    g_free (fret);
    return ret;
}


typedef struct {
    SearpcClient *client;
    AsyncCallback callback;
    const gchar *ret_type;
    int gtype;                /* to specify the specific gobject type 
                                 if ret_type is object or objlist */
    void *cbdata;
} AsyncCallData;

int
searpc_client_generic_callback (char *retstr, size_t len,
                                void *vdata, const char *errstr)
{
    AsyncCallData *data = vdata;
    GError *error = NULL;
    void *result = NULL;
    int ret;
    gint64 ret64;

    if (errstr) {
        g_set_error (&error, DFT_DOMAIN,
                     500, "Transport error: %s", errstr);
        data->callback (NULL, data->cbdata, error);
        g_error_free (error);
    } else {
        /* parse result and call the callback */
        if (strcmp(data->ret_type, "int") == 0) {
            ret = searpc_client_fret__int (retstr, len, &error);
            result = (void *)&ret;
        } if (strcmp(data->ret_type, "int64") == 0) {
            ret64 = searpc_client_fret__int64 (retstr, len, &error);
            result = (void *)&ret64;
        } else if (strcmp(data->ret_type, "string") == 0) {
            result = (void *)searpc_client_fret__string (retstr, len, &error);
        } else if (strcmp(data->ret_type, "object") == 0) {
            result = (void *)searpc_client_fret__object (
                data->gtype, retstr, len, &error);
        } else if (strcmp(data->ret_type, "objlist") == 0) {
            result = (void *)searpc_client_fret__objlist (
                data->gtype, retstr, len, &error);
        }

        data->callback (result, data->cbdata, error);

        if (strcmp(data->ret_type, "string") == 0) {
            g_free ((char *)result);
        } else if (strcmp(data->ret_type, "object") == 0) {
            if (result) g_object_unref ((GObject*)result);
        } else if (strcmp(data->ret_type, "objlist") == 0) {
            clean_objlist ((GList *)result);
        }
    }
    g_free (data);
}


int
searpc_client_async_call_v (SearpcClient *client,
                            const char *fname,
                            AsyncCallback callback,
                            const gchar *ret_type,
                            GType gtype,
                            void *cbdata,
                            int n_params,
                            va_list args)
{
    gsize len, ret_len;
    char *fstr;

    fstr = fcall_to_str (fname, n_params, args, &len);
    if (!fstr)
        return -1;
    
    int ret;
    AsyncCallData *data = g_new0(AsyncCallData, 1);
    data->client = client;
    data->callback = callback;
    data->ret_type = ret_type;
    data->gtype = gtype;
    data->cbdata = cbdata;

    ret = client->async_send (client->async_arg, fstr, len, data);
    if (ret < 0) {
        g_free (data);
        return -1;
    }
    return 0;
}

int
searpc_client_async_call__int (SearpcClient *client,
                               const char *fname,
                               AsyncCallback callback, void *cbdata,
                               int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, -1);

    va_list args;
    int ret;

    va_start (args, n_params);
    ret = searpc_client_async_call_v (client, fname, callback, "int", 0, cbdata,
                                      n_params, args);
    va_end (args);
    return ret;
}

int
searpc_client_async_call__int64 (SearpcClient *client,
                                 const char *fname,
                                 AsyncCallback callback, void *cbdata,
                                 int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, -1);

    va_list args;
    int ret;

    va_start (args, n_params);
    ret = searpc_client_async_call_v (client, fname, callback, "int64", 0, cbdata,
                                      n_params, args);
    va_end (args);
    return ret;
}

int
searpc_client_async_call__string (SearpcClient *client,
                                  const char *fname,
                                  AsyncCallback callback, void *cbdata,
                                  int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, -1);

    va_list args;
    int ret;

    va_start (args, n_params);
    ret = searpc_client_async_call_v (client, fname, callback, "string", 0, cbdata,
                                      n_params, args);
    va_end (args);
    return ret;
}

int
searpc_client_async_call__object (SearpcClient *client,
                                  const char *fname,
                                  AsyncCallback callback, 
                                  GType object_type, void *cbdata,
                                  int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, -1);

    va_list args;
    int ret;

    va_start (args, n_params);
    ret = searpc_client_async_call_v (client, fname, callback, "object",
                                      object_type, cbdata,
                                      n_params, args);
    va_end (args);
    return ret;
}

int
searpc_client_async_call__objlist (SearpcClient *client,
                                   const char *fname,
                                   AsyncCallback callback, 
                                   GType object_type, void *cbdata,
                                   int n_params, ...)
{
    g_return_val_if_fail (fname != NULL, -1);

    va_list args;
    int ret;

    va_start (args, n_params);
    ret = searpc_client_async_call_v (client, fname, callback, "objlist",
                                      object_type, cbdata,
                                      n_params, args);
    va_end (args);
    return ret;   
}


/*
 * Returns -1 if error happens in parsing data or data contains error
 * message. In this case, the calling function should simply return
 * to up layer.
 *
 * Returns 0 otherwise, and root is set to the root node, object set
 * to the root node's containing object.
 */
static int
handle_ret_common (char *data, size_t len, JsonParser **parser,
                   JsonNode **root,
                   JsonObject **object, GError **error)
{
    gint err_code;
    const gchar *err_msg;

    g_return_val_if_fail (root != 0 || object != 0, -1);

    *parser = json_parser_new ();
    if (!json_parser_load_from_data (*parser, data, len, error)) {
        g_object_unref (*parser);
        *parser = NULL;
        return -1;
    }

    *root = json_parser_get_root (*parser);
    *object = json_node_get_object (*root);
    if (*object == NULL) {
        g_set_error (error, DFT_DOMAIN,
                     502, "Invalid data: not a object");
        g_object_unref (*parser);
        *parser = NULL;
        *root = NULL;
        return -1;
    }

    if (json_object_has_member (*object, "err_code")) {
        err_code = json_object_get_int_member (*object, "err_code");
        err_msg = json_object_get_string_or_null_member (*object, "err_msg");
        g_set_error (error, DFT_DOMAIN,
                     err_code, "%s", err_msg);
        g_object_unref (*parser);
        *parser = NULL;
        *object = NULL;
        *root = NULL;
        return -1;
    }

    return 0;
}
    

char *
searpc_client_fret__string (char *data, size_t len, GError **error)
{
    JsonParser *parser = NULL;
    JsonObject *object = NULL;
    JsonNode   *root = NULL;
    gchar *ret_str = NULL;

    if (handle_ret_common(data, len, &parser, &root, &object, error) == 0) {
        ret_str = g_strdup (
            json_object_get_string_or_null_member (object, "ret"));
        g_object_unref (parser);
        return ret_str;
    }

    return NULL;
}

int
searpc_client_fret__int (char *data, size_t len, GError **error)
{
    JsonParser *parser = NULL;
    JsonNode   *root = NULL;
    JsonObject *object = NULL;
    int ret;

    if (handle_ret_common(data, len, &parser, &root, &object, error) == 0) {
        ret = (int) json_object_get_int_member(object, "ret");
        g_object_unref (parser);
        return ret;
    }

    return -1;
}

gint64
searpc_client_fret__int64 (char *data, size_t len, GError **error)
{
    JsonParser *parser = NULL;
    JsonNode   *root = NULL;
    JsonObject *object = NULL;
    gint64 ret;

    if (handle_ret_common(data, len, &parser, &root, &object, error) == 0) {
        ret = json_object_get_int_member(object, "ret");
        g_object_unref (parser);
        return ret;
    }

    return -1;
}

GObject*
searpc_client_fret__object (GType gtype, char *data, size_t len, GError **error)
{
    JsonParser *parser = NULL;
    JsonNode   *root = NULL;
    JsonObject *object = NULL;
    GObject    *ret = NULL;
    JsonNode   *member;

    if (handle_ret_common(data, len, &parser, &root, &object, error) == 0) {
        member = json_object_get_member (object, "ret");
        if (json_node_get_node_type(member) == JSON_NODE_NULL) {
            g_object_unref (parser);
            return NULL;
        }
        
        ret = json_gobject_deserialize(gtype, member);
        g_object_unref (parser);
        return ret;
    }

    return NULL;
}

GList*
searpc_client_fret__objlist (GType gtype, char *data, size_t len, GError **error)
{
    JsonParser *parser = NULL;
    JsonNode   *root = NULL;
    JsonObject *object = NULL;
    JsonArray  *array;
    JsonNode   *member;
    GList *ret = NULL;

    if (handle_ret_common(data, len, &parser, &root, &object, error) == 0) {
        member = json_object_get_member (object, "ret");
        if (json_node_get_node_type(member) == JSON_NODE_NULL) {
            g_object_unref (parser);
            return NULL;
        }

        array = json_node_get_array (member);
        g_assert (array);

        int i;
        for (i = 0; i < json_array_get_length(array); i++) {
            JsonNode *member = json_array_get_element (array, i);
            GObject *obj = json_gobject_deserialize(gtype, member);
            if (obj == NULL) {
                g_set_error (error, DFT_DOMAIN, 503, 
                             "Invalid data: object list contains null");
                clean_objlist(ret);
                g_object_unref (parser);
                return NULL;
            }
            ret = g_list_prepend (ret, obj);
        }
        g_object_unref (parser);
        return g_list_reverse(ret);
    }

    return NULL;
}
