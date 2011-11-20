/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <json-glib/json-glib.h>

#include "searpc-server.h"
#include "searpc-utils.h"

struct FuncItem;

typedef struct MarshalItem {
    SearpcMarshalFunc mfunc;
    gchar *signature;
} MarshalItem;

typedef struct FuncItem {
    void        *func;
    gchar       *fname;
    MarshalItem *marshal;
} FuncItem;


static void
func_item_free (FuncItem *item)
{
    g_free (item->fname);
    g_free (item);
}

static void
marshal_item_free (MarshalItem *item)
{
    g_free (item->signature);
    g_free (item);
}

static GHashTable *marshal_table;
static GHashTable *func_table;


/* Marshal functions */
static inline void
set_string_to_ret_object (JsonObject *object, gchar *ret)
{
    if (ret == NULL)
        json_object_set_null_member (object, "ret");
    else {
        json_object_set_string_member (object, "ret", ret);
        g_free (ret);
    }
}

static inline void
set_int_to_ret_object (JsonObject *object, gint64 ret)
{
    json_object_set_int_member (object, "ret", ret);
}

static inline void
set_object_to_ret_object (JsonObject *object, GObject *ret)
{
    if (ret == NULL)
        json_object_set_null_member (object, "ret");
    else {
        json_object_set_member (object, "ret", json_gobject_serialize(ret));
        g_object_unref (ret);
    }
}

static inline void
set_objlist_to_ret_object (JsonObject *object, GList *ret)
{
    GList *ptr;
    
    if (ret == NULL)
        json_object_set_null_member (object, "ret");
    else {
        JsonArray *array = json_array_new ();
        for (ptr = ret; ptr; ptr = ptr->next)
            json_array_add_element (array, json_gobject_serialize (ptr->data));
        json_object_set_array_member (object, "ret", array);

        for (ptr = ret; ptr; ptr = ptr->next)
            g_object_unref (ptr->data);
        g_list_free (ret);
    }
}

static gchar *
marshal_set_ret_common (JsonObject *object, gsize *len, GError *error)
{
    JsonNode *root = json_node_new (JSON_NODE_OBJECT);
    JsonGenerator *generator = json_generator_new ();
    gchar *data;

    if (error) {
        json_object_set_int_member (object, "err_code", error->code);
        json_object_set_string_or_null_member (object, "err_msg", error->message);
        g_error_free (error);
    }

    json_node_take_object (root, object);
    json_generator_set_root (generator, root);

    g_object_set (generator, "pretty", FALSE, NULL);
    data = json_generator_to_data (generator, len);

    json_node_free (root);
    g_object_unref (generator);
    return data;
}

/* include the generated marshal functions */
#include "marshal.h"

void
searpc_server_init ()
{
    func_table = g_hash_table_new_full (g_str_hash, g_str_equal, 
                                        NULL, (GDestroyNotify)func_item_free);
    marshal_table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           NULL, (GDestroyNotify)marshal_item_free);

    /* register buildin marshal functions */
    register_marshals(marshal_table);
}

void
searpc_server_final()
{
    g_hash_table_destroy (func_table);
    g_hash_table_destroy (marshal_table);
}

gboolean 
searpc_server_register_marshal (gchar *signature, SearpcMarshalFunc marshal)
{
    MarshalItem *mitem;

    g_assert (signature != NULL && marshal != NULL);

    if (g_hash_table_lookup (marshal_table, signature) != NULL) {
        g_warning ("[Sea RPC] cannot register duplicate marshal.\n");
        g_free (signature);
        return FALSE;
    }

    mitem = g_new0 (MarshalItem, 1);
    mitem->mfunc = marshal;
    mitem->signature = signature;
    g_hash_table_insert (marshal_table, (gpointer)mitem->signature, mitem);

    return TRUE;
}

gboolean 
searpc_server_register_function (void *func, const gchar *fname, gchar *signature)
{
    FuncItem *item;
    MarshalItem *mitem;

    g_assert (func != NULL && fname != NULL && signature != NULL);

    mitem = g_hash_table_lookup (marshal_table, signature);
    if (!mitem) {
        g_free (signature);
        return FALSE;
    }

    item = g_new0 (FuncItem, 1);
    item->marshal = mitem;
    item->fname = g_strdup(fname);
    item->func = func;

    g_hash_table_insert (func_table, (gpointer)item->fname, item);

    g_free (signature);
    return TRUE;
}

/* Called by RPC transport. */
gchar* 
searpc_server_call_function (gchar *func, gsize len, gsize *ret_len, GError **error)
{
    JsonParser *parser;
    JsonNode *root;
    JsonArray *array;

    g_return_val_if_fail (error == NULL || *error == NULL, NULL);
          
    parser = json_parser_new ();
    
    if (!json_parser_load_from_data (parser, func, len, error)) {
        g_warning ("[SeaRPC] failed to parse RPC call: %s\n", (*error)->message);
        g_object_unref (parser);
        return NULL;
    }

    root = json_parser_get_root (parser);
    array = json_node_get_array (root);

    const char *fname = json_array_get_string_element(array, 0);
    FuncItem *fitem = g_hash_table_lookup(func_table, fname);
    if (!fitem) {
        g_warning ("[SeaRPC] cannot find function %s.\n", fname);
        g_set_error (error, 0, 500, "cannot find function %s.", fname); 
        return NULL;
    }

    gchar* ret = fitem->marshal->mfunc (fitem->func, array, ret_len);

    g_object_unref (parser);

    return ret;
}

char* 
searpc_compute_signature(gchar *ret_type, int pnum, ...)
{
    va_list ap;
    int i = 0;
    char *ret;

    GChecksum *cksum = g_checksum_new (G_CHECKSUM_MD5);
    
    g_checksum_update (cksum, (const guchar*)ret_type, -1);
    
    va_start(ap, pnum);
    for (; i<pnum; i++) {
        char *ptype = va_arg (ap, char *);
        g_checksum_update (cksum, (const guchar*)":", -1);
        g_checksum_update (cksum, (const guchar*)ptype, -1);
    }
    va_end(ap);

    ret = g_strdup (g_checksum_get_string (cksum));
    g_checksum_free (cksum);

    return ret;
}
