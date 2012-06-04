#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>

#define DFT_DOMAIN g_quark_from_string("TEST")

#include "searpc-server.h"
#include "searpc-client.h"


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
SearpcClient *client;

char *
sample_send(void *arg, const gchar *fcall_str,
            size_t fcall_len, size_t *ret_len)
{
    g_assert (strcmp(arg, "test") == 0);

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
    g_assert (strcmp(arg, "test_async") == 0);
    
    char *ret;
    size_t ret_len;
    gchar *temp = g_strdup(fcall_str);

    ret = searpc_server_call_function ("test", temp, fcall_len, &ret_len);
    g_free (temp);

    searpc_client_generic_callback (ret, ret_len, rpc_priv, NULL);

    return 0;
}

void
init_sample_client ()
{
    client = searpc_client_new();
    client->send = sample_send;
    client->arg = "test";

    client->async_send = sample_async_send;
    client->async_arg = "test_async";
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

void test_simple_call (void *fixture, const void *data)
{
    gchar* result;
    GError *error = NULL;

    result = searpc_client_call__string (client, "get_substring", &error,
                                         2, "string", "hello", "int", 2);
    g_assert (error == NULL);
    g_assert (strcmp(result, "he") == 0);
    g_free (result);

    /* error should return */
    result = NULL;
    result = searpc_client_call__string (client, "get_substring", &error,
                                         2, "string", "hello", "int", 10);
    g_assert (error->message);
    g_free (result);
}

void
test_invalid_call (void *fixture, const void *data)
{
    char *fcall, *fret;
    gsize fcall_len, ret_len;
    gchar* result;
    GError *error = NULL;

    result = searpc_client_call__string (client, "nonexist_func", &error,
                                         2, "string", "hello", "int", 2);
    g_assert (error != NULL);
    g_free (result);
}

GObject *
get_maman_bar(const char *name, GError **error)
{
    return g_object_new(MAMAN_TYPE_BAR, "name", name, NULL);
}

void test_object_call (void *fixture, const void *data)
{
    GObject *result;
    GError *error = NULL;

    result = searpc_client_call__object (client, "get_maman_bar",
                                         MAMAN_TYPE_BAR, &error,
                                         1, "string", "kitty");
    g_assert (error == NULL);
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


void test_objlist_call (void *fixture, const void *data)
{
    GList *result, *ptr;
    GError *error = NULL;

    result = searpc_client_call__objlist (client, "get_maman_bar_list",
                                          MAMAN_TYPE_BAR, &error,
                                          2, "string", "kitty", "int", 10);
    g_assert (error == NULL);
    for (ptr = result; ptr; ptr = ptr->next)
        g_object_unref (ptr->data);
    g_list_free (result);


    result =  searpc_client_call__objlist (client, "get_maman_bar_list",
                                           MAMAN_TYPE_BAR, &error,
                                           2, "string", "kitty", "int", 0);
    g_assert (error == NULL);
    for (ptr = result; ptr; ptr = ptr->next)
        g_object_unref (ptr->data);
    g_list_free (result);
}

void simple_callback (void *result, void *user_data, GError *error)
{
    char *res = (char *)result;
    
    g_assert (strcmp(res, "he") == 0);
}

void simple_callback_error (void *result, void *user_data, GError *error)
{
    char *res = (char *)result;

    g_assert (result == NULL);
    g_assert (error != NULL);
}

void test_simple_call_async (void *fixture, const void *data)
{
    searpc_client_async_call__string (client, "get_substring",
                                      simple_callback, NULL,
                                      2, "string", "hello", "int", 2);

    searpc_client_async_call__string (client, "get_substring",
                                      simple_callback_error, NULL,
                                      2, "string", "hello", "int", 10);
}

int
main (int argc, char *argv[])
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    searpc_server_init ();
    searpc_create_service ("test");
    searpc_server_register_function ("test", get_substring, "get_substring", 
                                     searpc_signature_string__string_int());
    searpc_server_register_function ("test", get_maman_bar, "get_maman_bar", 
                                     searpc_signature_object__string());
    searpc_server_register_function ("test", get_maman_bar_list, "get_maman_bar_list", 
                                     searpc_signature_objlist__string_int());

    /* sample client */
    init_sample_client();

    g_test_add ("/searpc/simple", void, NULL,
                NULL, test_simple_call, NULL);

    /* test twice to detect memory error, for example, freed twice */
    g_test_add ("/searpc/simple2", void, NULL,
                NULL, test_simple_call, NULL);

    g_test_add ("/searpc/invalid_call", void, NULL,
                NULL, test_invalid_call, NULL);

    g_test_add ("/searpc/object", void, NULL,
                NULL, test_object_call, NULL);

    g_test_add ("/searpc/objlist", void, NULL,
                NULL, test_objlist_call, NULL);

    g_test_add ("/searpc/async/simple", void, NULL,
                NULL, test_simple_call_async, NULL);

    int ret = g_test_run();

    /* free memory for memory debug with valgrind */
    searpc_server_final();
    return ret;
}
