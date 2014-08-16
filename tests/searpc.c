#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>

#define DFT_DOMAIN g_quark_from_string("TEST")
#include <searpc.h>

#include "searpc-server.h"
#include "searpc-client.h"
#include "clar.h"

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

char *
sample_send(void *arg, const gchar *fcall_str,
            size_t fcall_len, size_t *ret_len)
{
    cl_assert (strcmp(arg, "test") == 0);

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
}

void
test_searpc__invalid_call (void)
{
    char *fcall, *fret;
    gsize fcall_len, ret_len;
    gchar* result;
    GError *error = NULL;

    result = searpc_client_call__string (client, "nonexist_func", &error,
                                         2, "string", "hello", "int", 2);
    cl_assert (error != NULL);
    g_free (result);
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

void simple_callback (void *result, void *user_data, GError *error)
{
    char *res = (char *)result;
    
    cl_assert (strcmp(res, "he") == 0);
}

void simple_callback_error (void *result, void *user_data, GError *error)
{
    char *res = (char *)result;

    cl_assert (result == NULL);
    cl_assert (error != NULL);
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

#define PAPA_NUMBER 5
#define PAPA_NUMBER_STR "5"
void
test_searpc__json_gobject_serialize (void)
{
    json_t *json, *j_name, *j_papa;
    gint papa_number = PAPA_NUMBER;
    gchar name[] = "test";
    MamanBar *bar = g_object_new (MAMAN_TYPE_BAR,
                                  "name", g_strdup(name),
                                  "papa-number", papa_number,
                                  NULL);
    json = json_gobject_serialize ((GObject*)bar);
    j_name = json_object_get (json, "name");
    j_papa = json_object_get (json, "papa-number");
    cl_assert (json_is_string (j_name));
    cl_assert (json_is_integer (j_papa));
    cl_assert (strcmp (json_string_value (j_name), "test") == 0);
    cl_assert (json_integer_value (j_papa) == PAPA_NUMBER);
    json_decref (json);
    g_object_unref (bar);
}

void
test_searpc__json_gobject_deserialize (void)
{
    gchar *name;
    gint papa_number;
    json_error_t error;
    json_t* json = json_loads ("{\"name\":\"test\",\"papa-number\":"PAPA_NUMBER_STR"}", 0, &error);
    MamanBar *bar = (MamanBar*)json_gobject_deserialize (MAMAN_TYPE_BAR, json);
    g_object_get (bar,
                  "name", &name,
                  "papa-number", &papa_number,
                  NULL);
    cl_assert (name != NULL);
    cl_assert (strcmp(name, "test") == 0);
    cl_assert (papa_number == PAPA_NUMBER);
    g_free (name);
    json_decref (json);
    g_object_unref (bar);
}

#include "searpc-signature.h"
#include "searpc-marshal.h"

void
test_searpc__initialize (void)
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init ();
#endif
    searpc_server_init (register_marshals);
    searpc_create_service ("test");
    searpc_server_register_function ("test", get_substring, "get_substring", 
                                     searpc_signature_string__string_int());
    searpc_server_register_function ("test", get_maman_bar, "get_maman_bar", 
                                     searpc_signature_object__string());
    searpc_server_register_function ("test", get_maman_bar_list, "get_maman_bar_list", 
                                     searpc_signature_objlist__string_int());

    /* sample client */
    client = searpc_client_new();
    client->send = sample_send;
    client->arg = "test";

    client->async_send = sample_async_send;
    client->async_arg = "test_async";
}

void
test_searpc__cleanup (void)
{
    /* free memory for memory debug with valgrind */
    searpc_server_final();
}
