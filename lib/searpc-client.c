#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <json-glib/json-glib.h>

#include "searpc-client.h"
#include "searpc-utils.h"

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
    return client->transport(client->arg, fcall_str,
                             fcall_len, ret_len);
}

typedef struct {
    SearpcClient *client;
    AsyncCallback callback;
    const gchar *ret_type;
    int gtype;
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
        g_set_error (&error, 0, 500, "Transport error: %s", errstr);
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
    }
    g_free (data);
}

int
searpc_client_async_call (SearpcClient *client,
                          gchar *fcall_str,
                          size_t fcall_len,
                          AsyncCallback callback,
                          const gchar *ret_type,
                          int gtype,
                          void *cbdata)
{
    int ret;
    AsyncCallData *data = g_new0(AsyncCallData, 1);
    data->client = client;
    data->callback = callback;
    data->ret_type = ret_type;
    data->gtype = gtype;
    data->cbdata = cbdata;

    ret = client->async_send (client->async_arg, fcall_str, fcall_len, data);
    if (ret < 0) {
        g_free (data);
        return -1;
    }
    return 0;
}

/*
 * serialize function call from array to string
 */
static char *
fcall_common (JsonArray *array, gsize *len)
{
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

#include "fcall-impr.h"


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
        g_set_error (error, 0, 502, "Invalid data: not a object");
        g_object_unref (*parser);
        *parser = NULL;
        *root = NULL;
        return -1;
    }

    if (json_object_has_member (*object, "err_code")) {
        err_code = json_object_get_int_member (*object, "err_code");
        err_msg = json_object_get_string_or_null_member (*object, "err_msg");
        g_set_error (error, 0, err_code, "%s", err_msg);
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


static void clean_objlist(GList *list)
{
    GList *ptr;
    for (ptr = list; ptr; ptr = ptr->next)
        g_object_unref(ptr->data);
    g_list_free (list);
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
                g_set_error (error, 0, 503, 
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
