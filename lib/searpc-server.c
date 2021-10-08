/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <jansson.h>

#include "searpc-server.h"
#include "searpc-utils.h"

#ifdef __linux__
#include <sys/time.h>
#include <sys/errno.h>
#include <pthread.h>
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

#ifdef __linux__
static FILE *slow_log_fp = NULL;
static gint64 slow_threshold;
static pthread_mutex_t slow_log_lock;
#endif

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
void
searpc_set_string_to_ret_object (json_t *object, char *ret)
{
    if (ret == NULL)
        json_object_set_new (object, "ret", json_null ());
    else {
        json_object_set_new (object, "ret", json_string (ret));
        g_free (ret);
    }
}

void
searpc_set_int_to_ret_object (json_t *object, json_int_t ret)
{
    json_object_set_new (object, "ret", json_integer (ret));
}

void
searpc_set_object_to_ret_object (json_t *object, GObject *ret)
{
    if (ret == NULL)
        json_object_set_new (object, "ret", json_null ());
    else {
        json_object_set_new (object, "ret", json_gobject_serialize (ret));
        g_object_unref (ret);
    }
}

void
searpc_set_objlist_to_ret_object (json_t *object, GList *ret)
{
    GList *ptr;
    
    if (ret == NULL)
        json_object_set_new (object, "ret", json_null ());
    else {
        json_t *array = json_array ();
        for (ptr = ret; ptr; ptr = ptr->next)
            json_array_append_new (array, json_gobject_serialize (ptr->data));
        json_object_set_new (object, "ret", array);

        for (ptr = ret; ptr; ptr = ptr->next)
            g_object_unref (ptr->data);
        g_list_free (ret);
    }
}

void
searpc_set_json_to_ret_object (json_t *object, json_t *ret)
{
    if (ret == NULL)
        json_object_set_new(object, "ret", json_null ());
    else
        json_object_set_new (object, "ret", ret);
}

char *
searpc_marshal_set_ret_common (json_t *object, gsize *len,  GError *error)
{

    char *data;

    if (error) {
        json_object_set_new (object, "err_code", json_integer((json_int_t)error->code));
        json_object_set_new (object, "err_msg", json_string(error->message));
        g_error_free (error);
    }

    data=json_dumps(object,JSON_COMPACT);
    *len=strlen(data);
    json_decref(object);

    return data;
}

char *
error_to_json (int code, const char *msg, gsize *len)
{
    json_t *object = json_object ();
    char *data;

    json_object_set_new (object, "err_code", json_integer((json_int_t)code));
    json_object_set_string_or_null_member(object, "err_msg", msg);

    data=json_dumps(object,JSON_COMPACT);
    *len=strlen(data);
    json_decref(object);

    return data;
}

void
searpc_server_init (RegisterMarshalFunc register_func)
{
    marshal_table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           NULL, (GDestroyNotify)marshal_item_free);
    service_table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           NULL, (GDestroyNotify)service_free);

    register_func ();
}

#ifdef __linux__

int
searpc_server_init_with_slow_log (RegisterMarshalFunc register_func,
                                  const char *slow_log_path,
                                  gint64 slow_threshold_in)
{
    if (slow_log_path) {
        slow_log_fp = fopen (slow_log_path, "a+");
        if (!slow_log_fp) {
            g_warning ("Failed to open RPC slow log file %s: %s\n", slow_log_path, strerror(errno));
            return -1;
        }
        slow_threshold = slow_threshold_in;

        pthread_mutex_init (&slow_log_lock, NULL);
    }

    searpc_server_init (register_func);

    return 0;
}

int
searpc_server_reopen_slow_log (const char *slow_log_path)
{
    FILE *fp, *oldfp;

    if ((fp = fopen (slow_log_path, "a+")) == NULL) {
        g_warning ("Failed to open RPC slow log file %s\n", slow_log_path);
        return -1;
    }

    pthread_mutex_lock (&slow_log_lock);
    oldfp = slow_log_fp;
    slow_log_fp = fp;
    pthread_mutex_unlock (&slow_log_lock);

    if (fclose(oldfp) < 0) {
        g_warning ("Failed to close old RPC slow log file\n");
        return -1;
    }

    return 0;
}

#endif

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

#ifdef __linux__

static void
print_slow_log_if_necessary (const char *svc_name, const char *func, gsize len,
                             const struct timeval *start,
                             const struct timeval *intv)
{
    char time_buf[64];
    gint64 intv_in_usec = ((gint64)intv->tv_sec) * G_USEC_PER_SEC + (gint64)intv->tv_usec;
    gint64 intv_in_msec = intv_in_usec/1000;
    double intv_in_sec = ((double)intv_in_usec)/G_USEC_PER_SEC;

    if (intv_in_msec < slow_threshold)
        return;

    strftime(time_buf, 64, "%Y/%m/%d %H:%M:%S", localtime(&start->tv_sec));

    pthread_mutex_lock (&slow_log_lock);

    fprintf (slow_log_fp, "%s - %s - %.*s - %.3f\n",
             time_buf, svc_name, (int)len, func, intv_in_sec);
    fflush (slow_log_fp);

    pthread_mutex_unlock (&slow_log_lock);
}

#endif

/* Called by RPC transport. */
char* 
searpc_server_call_function (const char *svc_name,
                             gchar *func, gsize len, gsize *ret_len)
{
    SearpcService *service;
    json_t *array;
    char* ret;
    json_error_t jerror;
    GError *error = NULL;

#ifdef __linux__
    struct timeval start, end, intv;

    if (slow_log_fp) {
        gettimeofday(&start, NULL);
    }
#endif

    service = g_hash_table_lookup (service_table, svc_name);
    if (!service) {
        char buf[256];
        snprintf (buf, 255, "cannot find service %s.", svc_name);
        return error_to_json (501, buf, ret_len);
    }
    
    array = json_loadb (func, len, 0 ,&jerror);
    
    if (!array) {
        char buf[512];
        setjetoge(&jerror,&error);
        snprintf (buf, 511, "failed to load RPC call: %s\n", error->message);
        json_decref (array);        
        g_error_free(error);
        return error_to_json (511, buf, ret_len);
    }

    const char *fname = json_string_value (json_array_get(array, 0));
    FuncItem *fitem = g_hash_table_lookup(service->func_table, fname);
    if (!fitem) {
        char buf[256];
        snprintf (buf, 255, "cannot find function %s.", fname);
        json_decref (array);
        return error_to_json (500, buf, ret_len);
    }

    ret = fitem->marshal->mfunc (fitem->func, array, ret_len);

#ifdef __linux__
    if (slow_log_fp) {
        gettimeofday(&end, NULL);
        timersub(&end, &start, &intv);
        print_slow_log_if_necessary (svc_name, func, len, &start, &intv);
    }
#endif

    json_decref(array);

    return ret;
}

char* 
searpc_compute_signature(const gchar *ret_type, int pnum, ...)
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
