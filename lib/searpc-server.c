/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "json-glib/json-glib.h"

#include "searpc-server.h"
#include "searpc-utils.h"

#ifdef PROFILE
    #include <sys/time.h>
#endif

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

typedef struct {
    char *name;
    GHashTable *func_table;
} SearpcService;

static GHashTable *marshal_table;
static GHashTable *service_table;

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

int
searpc_create_service (const char *svc_name)
{
    SearpcService *service;

    if (!svc_name)
        return -1;

    if (g_hash_table_lookup (service_table, svc_name) != NULL)
        return 0;

    service = g_new0 (SearpcService, 1);
    service->name = g_strdup(svc_name);
    service->func_table = g_hash_table_new_full (g_str_hash, g_str_equal, 
                                                 NULL, (GDestroyNotify)func_item_free);

    g_hash_table_insert (service_table, service->name, service);

    return 0;
}

static void
service_free (SearpcService *service)
{
    g_free (service->name);
    g_hash_table_destroy (service->func_table);
    g_free (service);
}

void
searpc_remove_service (const char *svc_name)
{
    if (!svc_name)
        return;
    g_hash_table_remove (service_table, svc_name);
}

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

static gchar *
error_to_json (int code, const char *msg, gsize *len)
{
    JsonObject *object = json_object_new ();
    JsonNode *root = json_node_new (JSON_NODE_OBJECT);
    JsonGenerator *generator = json_generator_new ();
    gchar *data;

    json_object_set_int_member (object, "err_code", code);
    json_object_set_string_or_null_member (object, "err_msg", msg);
    
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
    marshal_table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           NULL, (GDestroyNotify)marshal_item_free);
    service_table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           NULL, (GDestroyNotify)service_free);

    /* register buildin marshal functions */
    register_marshals(marshal_table);
}

void
searpc_server_final()
{
    g_hash_table_destroy (service_table);
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
searpc_server_register_function (const char *svc_name,
                                 void *func, const gchar *fname, gchar *signature)
{
    SearpcService *service;
    FuncItem *item;
    MarshalItem *mitem;

    g_assert (svc_name != NULL && func != NULL && fname != NULL && signature != NULL);

    service = g_hash_table_lookup (service_table, svc_name);
    if (!service)
        return FALSE;

    mitem = g_hash_table_lookup (marshal_table, signature);
    if (!mitem) {
        g_free (signature);
        return FALSE;
    }

    item = g_new0 (FuncItem, 1);
    item->marshal = mitem;
    item->fname = g_strdup(fname);
    item->func = func;

    g_hash_table_insert (service->func_table, (gpointer)item->fname, item);

    g_free (signature);
    return TRUE;
}

/* Called by RPC transport. */
gchar* 
searpc_server_call_function (const char *svc_name,
                             gchar *func, gsize len, gsize *ret_len)
{
    SearpcService *service;
    JsonParser *parser;
    JsonNode *root;
    JsonArray *array;
    gchar* ret;
    GError *error = NULL;
#ifdef PROFILE
    struct timeval start, end, intv;

    gettimeofday(&start, NULL);
#endif

    service = g_hash_table_lookup (service_table, svc_name);
    if (!service) {
        char buf[256];
        snprintf (buf, 255, "cannot find service %s.", svc_name);
        return error_to_json (501, buf, ret_len);
    }
          
    parser = json_parser_new ();
    
    if (!json_parser_load_from_data (parser, func, len, &error)) {
        char buf[512];
        snprintf (buf, 511, "failed to parse RPC call: %s\n", error->message);
        g_object_unref (parser);        
        return error_to_json (511, buf, ret_len);
    }

    root = json_parser_get_root (parser);
    array = json_node_get_array (root);

    const char *fname = json_array_get_string_element(array, 0);
    FuncItem *fitem = g_hash_table_lookup(service->func_table, fname);
    if (!fitem) {
        char buf[256];
        snprintf (buf, 255, "cannot find function %s.", fname);
        g_object_unref (parser);
        return error_to_json (500, buf, ret_len);
    }

    ret = fitem->marshal->mfunc (fitem->func, array, ret_len);

#ifdef PROFILE
    gettimeofday(&end, NULL);
    timersub(&end, &start, &intv);
    g_debug ("[searpc] Time spend in call %s: %ds %dus\n",
             fname, intv.tv_sec, intv.tv_usec);
#endif

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
