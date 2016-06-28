#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>

#define DFT_DOMAIN g_quark_from_string("TEST")
#include <searpc.h>

#include "searpc-server.h"
#include "searpc-client.h"
#include "searpc-named-pipe-transport.h"
#include "clar.h"

#if !defined(WIN32)
static const char *pipe_path = "/tmp/.searpc-test";
#else
static const char *pipe_path = "\\\\.\\pipe\\libsearpc-test";
#endif

/* sample class */

#define MAMAN_TYPE_BAR                  (maman_bar_get_type ())
#define MAMAN_BAR(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MAMAN_TYPE_BAR, MamanBar))
#define MAMAN_IS_BAR(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAMAN_TYPE_BAR))
#define MAMAN_BAR_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MAMAN_TYPE_BAR, MamanBarClass))
#define MAMAN_IS_BAR_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MAMAN_TYPE_BAR))
#define MAMAN_BAR_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MAMAN_TYPE_BAR, MamanBarClass))

typedef struct _MamanBar        MamanBar;
typedef struct _MamanBarClass   MamanBarClass;

struct _MamanBar
{
    GObject parent_instance;

    gchar *name;
    int    papa_number;
};

struct _MamanBarClass
{
    GObjectClass parent_class;
};

G_DEFINE_TYPE (MamanBar, maman_bar, G_TYPE_OBJECT);

enum
{
  PROP_0,
  PROP_MAMAN_NAME,
  PROP_PAPA_NUMBER
};

static void
maman_bar_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    MamanBar *self = MAMAN_BAR (object);

    switch (property_id) {
    case PROP_MAMAN_NAME:
        g_free (self->name);
        self->name = g_value_dup_string (value);
        break;

    case PROP_PAPA_NUMBER:
        self->papa_number = g_value_get_uchar (value);
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
maman_bar_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    MamanBar *self = MAMAN_BAR (object);

    switch (property_id) {
    case PROP_MAMAN_NAME:
        g_value_set_string (value, self->name);
        break;

    case PROP_PAPA_NUMBER:
        g_value_set_uchar (value, self->papa_number);
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
maman_bar_finalize (GObject *gobject)
{
  MamanBar *self = MAMAN_BAR (gobject);

  g_free (self->name);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (maman_bar_parent_class)->finalize (gobject);
}

static void
maman_bar_class_init (MamanBarClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->set_property = maman_bar_set_property;
    gobject_class->get_property = maman_bar_get_property;
    gobject_class->finalize = maman_bar_finalize;

    pspec = g_param_spec_string ("name",
                                 "Maman name",
                                 "Set maman's name",
                                 "no-name-set" /* default value */,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_MAMAN_NAME,
                                     pspec);

    pspec = g_param_spec_uchar ("papa-number",
                                "Number of current Papa",
                                "Set/Get papa's number",
                                0  /* minimum value */,
                                10 /* maximum value */,
                                2  /* default value */,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_PAPA_NUMBER,
                                     pspec);
}

static void
maman_bar_init (MamanBar *self)
{

}

/* sample client */
static SearpcClient *client;
/* sample client with named pipe as transport */
static SearpcClient *client_with_pipe_transport;

char *
sample_send(void *arg, const gchar *fcall_str,
            size_t fcall_len, size_t *ret_len)
{
    cl_assert_ (strcmp(arg, "test") == 0, arg);

    char *ret;
    /* directly call in memory, instead of send via network */
    gchar *temp = g_strdup(fcall_str);
    ret = searpc_server_call_function ("test", temp, fcall_len, ret_len);
    g_free (temp);
    return ret;
}

int
sample_async_send (void *arg, gchar *fcall_str,
                   size_t fcall_len, void *rpc_priv)
{
    cl_assert (strcmp(arg, "test_async") == 0);

    char *ret;
    size_t ret_len;
    gchar *temp = g_strdup(fcall_str);

    ret = searpc_server_call_function ("test", temp, fcall_len, &ret_len);
    g_free (temp);

    searpc_client_generic_callback (ret, ret_len, rpc_priv, NULL);

    g_free (ret);
    return 0;
}

gchar *
get_substring (const gchar *orig_str, int sub_len, GError **error)
{
    if (sub_len > strlen(orig_str)) {
        g_set_error (error, DFT_DOMAIN, 100,
                     "Substring length larger than the length of origin string");
        return NULL;
    }
    gchar *ret = g_malloc0(sub_len+1);
    memcpy(ret, orig_str, sub_len);
    ret[sub_len] = '\0';
    return ret;
}

static SearpcClient *
do_create_client_with_pipe_transport()
{
    SearpcNamedPipeClient *pipe_client = searpc_create_named_pipe_client(pipe_path);
    cl_must_pass_(searpc_named_pipe_client_connect(pipe_client), "named pipe client failed to connect");
    return searpc_client_with_named_pipe_transport(pipe_client, "test");
}


void
test_searpc__simple_call (void)
{
    gchar* result;
    GError *error = NULL;

    result = searpc_client_call__string (client, "get_substring", &error,
                                         2, "string", "hello", "int", 2);
    cl_assert (error == NULL);
    cl_assert (strcmp(result, "he") == 0);
    g_free (result);

    /* error should return */
    result = NULL;
    result = searpc_client_call__string (client, "get_substring", &error,
                                         2, "string", "hello", "int", 10);
    cl_assert (error->message);
    g_free (result);
    g_error_free(error);
}

void
test_searpc__invalid_call (void)
{
    gchar* result;
    GError *error = NULL;

    result = searpc_client_call__string (client, "nonexist_func", &error,
                                         2, "string", "hello", "int", 2);
    cl_assert (error != NULL);
    g_free (result);
    g_error_free (error);
}

GObject *
get_maman_bar(const char *name, GError **error)
{
    return g_object_new(MAMAN_TYPE_BAR, "name", name, NULL);
}

void
test_searpc__object_call (void)
{
    GObject *result;
    GError *error = NULL;

    result = searpc_client_call__object (client, "get_maman_bar",
                                         MAMAN_TYPE_BAR, &error,
                                         1, "string", "kitty");
    cl_assert (error == NULL);
    g_object_unref (result);
}

GList *
get_maman_bar_list (const char *name, int num, GError **error)
{
    char buf[256];
    GList *ret = 0;
    int i;
    GObject *obj;

    if (num < 0) {
        g_set_error (error, DFT_DOMAIN, 100, "num must be positive.");
        return NULL;
    }
    if (num > 1000) {
        g_set_error (error, DFT_DOMAIN, 100, "num must no larger than 1000.");
        return NULL;
    }

    for (i = 0; i < num; i++) {
        sprintf (buf, "%s%d", name, i);
        obj = g_object_new(MAMAN_TYPE_BAR, "name", buf, NULL);
        ret = g_list_prepend (ret, obj);
    }
    ret = g_list_reverse (ret);
    return ret;
}


void
test_searpc__objlist_call (void)
{
    GList *result, *ptr;
    GError *error = NULL;

    result = searpc_client_call__objlist (client, "get_maman_bar_list",
                                          MAMAN_TYPE_BAR, &error,
                                          2, "string", "kitty", "int", 10);
    cl_assert (error == NULL);
    for (ptr = result; ptr; ptr = ptr->next)
        g_object_unref (ptr->data);
    g_list_free (result);

    result =  searpc_client_call__objlist (client, "get_maman_bar_list",
                                           MAMAN_TYPE_BAR, &error,
                                           2, "string", "kitty", "int", 0);
    cl_assert (error == NULL);
    for (ptr = result; ptr; ptr = ptr->next)
        g_object_unref (ptr->data);
    g_list_free (result);
}

json_t *
simple_json_rpc (const char *name, int num, GError **error)
{
    json_t * ret = json_object();
    json_object_set_new (ret, name, json_integer (num));
    return ret;
}

void
test_searpc__json_return_type (void)
{
    json_t *result;
    GError *error = NULL;

    result = searpc_client_call__json (client, "simple_json_rpc",
                                       &error, 2,
                                       "string", "year",
                                       "int", 2016);
    cl_assert (error == NULL);
    cl_assert (json_integer_value(json_object_get(result, "year")) == 2016);
    json_decref(result);
}

// The macro `json_object_foreach` is not defined in older versions of
// libjansson.
#ifndef json_object_foreach
#define json_object_foreach(object, key, value) \
    for(key = json_object_iter_key(json_object_iter(object)); \
        key && (value = json_object_iter_value(json_object_key_to_iter(key))); \
        key = json_object_iter_key(json_object_iter_next(object, json_object_key_to_iter(key))))
#endif

json_t *
count_json_kvs (const json_t *obj, GError **error)
{
    int count = 0;
    const char *key;
    const json_t *value;
    json_object_foreach(((json_t*)obj), key, value) {
        count++;
    }

    json_t * ret = json_object();
    json_object_set_new (ret, "number_of_kvs", json_integer (count));
    return ret;
}

void
test_searpc__json_param_type (void)
{
    json_t *result;
    GError *error = NULL;

    json_t *param = json_object();
    json_object_set_new (param, "a", json_integer (1));
    json_object_set_new (param, "b", json_integer (2));

    result = searpc_client_call__json (client, "count_json_kvs",
                                       &error, 1,
                                       "json", param);

    cl_assert_ (error == NULL, error ? error->message : "");
    int count = json_integer_value(json_object_get((json_t*)result, "number_of_kvs"));
    char *msg = json_dumps(result, JSON_INDENT(2));
    cl_assert_(count == 2, msg);
    free (msg);
    json_decref(param);
    json_decref(result);
}


void simple_callback (void *result, void *user_data, GError *error)
{
    char *res = (char *)result;

    cl_assert (strcmp(res, "he") == 0);
}

void simple_callback_error (void *result, void *user_data, GError *error)
{
    cl_assert (result == NULL);
    cl_assert (error != NULL);
    g_error_free (error);
}

void
test_searpc__simple_call_async (void)
{
    searpc_client_async_call__string (client, "get_substring",
                                      simple_callback, NULL,
                                      2, "string", "hello", "int", 2);

    searpc_client_async_call__string (client, "get_substring",
                                      simple_callback_error, NULL,
                                      2, "string", "hello", "int", 10);
}

void async_callback_json (void *result, void *user_data, GError *error)
{
    cl_assert(json_integer_value(json_object_get((json_t*)result, "hello")) == 10);
}

void
test_searpc__simple_call_async_json (void)
{
    searpc_client_async_call__json (client, "simple_json_rpc",
                                    async_callback_json, NULL,
                                    2, "string", "hello", "int", 10);
}

void
test_searpc__pipe_simple_call (void)
{
    gchar* result;
    GError *error = NULL;

    result = searpc_client_call__string (client_with_pipe_transport, "get_substring", &error,
                                         2, "string", "hello", "int", 2);
    cl_assert_ (error == NULL, error ? error->message : "");
    cl_assert (strcmp(result, "he") == 0);
    g_free (result);

    /* error should return */
    result = searpc_client_call__string (client_with_pipe_transport, "get_substring", &error,
                                         2, "string", "hello", "int", 10);
    cl_assert (error->message);
    g_free (result);
}

void
test_searpc__pipe_large_request (void)
{
    gchar* result;
    GError *error = NULL;

    // 10MB
    int size = 10 * 1024 * 1024;
    GString *large_string = g_string_sized_new(size);
    while (large_string->len < size) {
        g_string_append(large_string, "aaaa");
    }

    // Large request
    result = searpc_client_call__string (client_with_pipe_transport, "get_substring", &error,
                                         2, "string", large_string->str, "int", 2);
    cl_assert_ (error == NULL, error ? error->message : "");
    cl_assert_ (strcmp(result, "aa") == 0, result);
    g_free (result);

    // Large request & Large response
    result = searpc_client_call__string (client_with_pipe_transport, "get_substring", &error,
                                         2, "string", large_string->str, "int", size - 2);
    cl_assert_ (error == NULL, error ? error->message : "");
    // cl_assert (strcmp(result, "aa") == 0);
    g_free (result);

    g_string_free (large_string, TRUE);
}

static void * do_pipe_connect_and_request(void *arg)
{
    SearpcClient *client = do_create_client_with_pipe_transport();

    // 100KB
    int size = 100 * 1024;
    GString *large_string = g_string_sized_new(size);
    while (large_string->len < size) {
        g_string_append(large_string, "aaaa");
    }

    gchar* result;
    GError *error = NULL;
    result = searpc_client_call__string (client, "get_substring", &error,
                                         2, "string", large_string->str, "int", 2);
    cl_assert_ (error == NULL, error ? error->message : "");
    cl_assert_ (strcmp(result, "aa") == 0, result);
    g_free (result);

    g_string_free (large_string, TRUE);
    searpc_free_client_with_pipe_transport(client);

    return NULL;
}

// Simulate the situation that the server can handle multiple clients connecting
// at the same time.
void
test_searpc__pipe_concurrent_clients (void)
{
    // M concurrent clients, and run the test for N times.
    int m_clients = 5;
    int n_times = 20;

    int i;
    for (i = 0; i < n_times; i++) {
        g_usleep(100000);
        pthread_t *threads = g_new0(pthread_t, m_clients);

        int j;
        for (j = 0; j < m_clients; j++) {
            pthread_create(&threads[j], NULL, do_pipe_connect_and_request, NULL);
        }

        void *ret;
        for (j = 0; j < m_clients; j++) {
            pthread_join(threads[j], &ret);
        }
        g_free (threads);
    }
}


#include "searpc-signature.h"
#include "searpc-marshal.h"

void
test_searpc__initialize (void)
{
    searpc_server_init (register_marshals);
    searpc_create_service ("test");
    searpc_server_register_function ("test", get_substring, "get_substring",
                                     searpc_signature_string__string_int());
    searpc_server_register_function ("test", get_maman_bar, "get_maman_bar",
                                     searpc_signature_object__string());
    searpc_server_register_function ("test", get_maman_bar_list, "get_maman_bar_list",
                                     searpc_signature_objlist__string_int());
    searpc_server_register_function ("test", simple_json_rpc, "simple_json_rpc",
                                     searpc_signature_json__string_int());
    searpc_server_register_function ("test", count_json_kvs, "count_json_kvs",
                                     searpc_signature_json__json());

    /* sample client */
    client = searpc_client_new();
    client->send = sample_send;
    client->arg = "test";

    client->async_send = sample_async_send;
    client->async_arg = "test_async";

    SearpcNamedPipeServer *pipe_server = searpc_create_named_pipe_server(pipe_path);
    cl_must_pass_(searpc_named_pipe_server_start(pipe_server), "named pipe server failed to start");
#if defined(WIN32)
    // Wait for the server thread to start
    Sleep(1000);
#endif

    client_with_pipe_transport = do_create_client_with_pipe_transport();
}

void
test_searpc__cleanup (void)
{
    searpc_free_client_with_pipe_transport(client_with_pipe_transport);

    /* free memory for memory debug with valgrind */
    searpc_server_final();
}
