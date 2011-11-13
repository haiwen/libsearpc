
#include <glib.h>
#include <string.h>

#include "searpc-server.h"
#include "searpc-client.h"

#include <glib-object.h>

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
        g_print ("maman: %s\n", self->name);
        break;

    case PROP_PAPA_NUMBER:
        self->papa_number = g_value_get_uchar (value);
        g_print ("papa: %u\n", self->papa_number);
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

/* test function define macro */
SEARPC_CLIENT_DEFUN_INT__STRING(func4);
SEARPC_CLIENT_DEFUN_INT__STRING_STRING(func1);
SEARPC_CLIENT_DEFUN_STRING__VOID(func0);
SEARPC_CLIENT_DEFUN_OBJECT__STRING(func2, 0);
SEARPC_CLIENT_DEFUN_OBJLIST__STRING_INT(func3, 0);


gchar *
get_substring (const gchar *orig_str, int sub_len, GError **error)
{
    if (sub_len > strlen(orig_str)) {
        g_set_error (error, 0, 100,
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
    char *fcall, *fret;
    gsize fcall_len, ret_len;
    gchar* result;
    GError *error = NULL;

    fcall = searpc_client_fcall__string_int ("get_substring", "hello", 2,
                                             &fcall_len);
    fret = searpc_server_call_function (fcall, fcall_len, &ret_len);
    result = searpc_client_fret__string (fret, ret_len, &error);
    g_assert (strcmp(result, "he") == 0);
    g_free (fcall);
    g_free (fret);
    g_free (result);

    /* error should return */
    result = NULL;
    fcall = searpc_client_fcall__string_int ("get_substring", "hello", 7,
                                             &fcall_len);
    fret = searpc_server_call_function (fcall, fcall_len, &ret_len);
    result = searpc_client_fret__string (fret, ret_len, &error);
    g_assert (error->message);
    g_free (fcall);
    g_free (fret);
    g_free (result);
}

GObject *
get_maman_bar(const char *name, GError **error)
{
    return g_object_new(MAMAN_TYPE_BAR, "name", name, NULL);
}

void test_object_call (void *fixture, const void *data)
{
    char *fcall, *fret;
    gsize fcall_len, ret_len;
    GObject *result;
    GError *error = NULL;

    fcall = searpc_client_fcall__string ("get_maman_bar", "kitty",
                                         &fcall_len);
    fret = searpc_server_call_function (fcall, fcall_len, &ret_len);
    result = searpc_client_fret__object (MAMAN_TYPE_BAR, fret, ret_len, &error);

    g_free (fcall);
    g_free (fret);
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
        g_set_error (error, 0, 100, "num must be positive.");
        return NULL;
    }
    if (num > 1000) {
        g_set_error (error, 0, 100, "num must no larger than 1000.");
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
    char *fcall, *fret;
    gsize fcall_len, ret_len;
    GList *result, *ptr;
    GError *error = NULL;

    fcall = searpc_client_fcall__string_int ("get_maman_bar_list", "kitty", 10,
                                             &fcall_len);
    fret = searpc_server_call_function (fcall, fcall_len, &ret_len);
    result = searpc_client_fret__objlist (MAMAN_TYPE_BAR, fret, ret_len, &error);
    g_free (fcall);
    g_free (fret);
    for (ptr = result; ptr; ptr = ptr->next)
        g_object_unref (ptr->data);
    g_list_free (result);


    fcall = searpc_client_fcall__string_int ("get_maman_bar_list", "kitty", 0,
                                             &fcall_len);
    fret = searpc_server_call_function (fcall, fcall_len, &ret_len);
    result = searpc_client_fret__objlist (MAMAN_TYPE_BAR, fret, ret_len, &error);
    g_free (fcall);
    g_free (fret);
    for (ptr = result; ptr; ptr = ptr->next)
        g_object_unref (ptr->data);
    g_list_free (result);
}


int
main (int argc, char *argv[])
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    searpc_server_init ();
    searpc_server_register_function (get_substring, "get_substring", 
                                     searpc_signature_string__string_int());
    searpc_server_register_function (get_maman_bar, "get_maman_bar", 
                                     searpc_signature_object__string());
    searpc_server_register_function (get_maman_bar_list, "get_maman_bar_list", 
                                     searpc_signature_objlist__string_int());

    g_test_add ("/searpc/simple", void, NULL,
                NULL, test_simple_call, NULL);

    /* test twice to detect memory error, for example, freed twice */
    g_test_add ("/searpc/simple2", void, NULL,
                NULL, test_simple_call, NULL);

    g_test_add ("/searpc/object", void, NULL,
                NULL, test_object_call, NULL);

    g_test_add ("/searpc/objlist", void, NULL,
                NULL, test_objlist_call, NULL);

    int ret = g_test_run();

    /* free memory for memory debug with valgrind */
    searpc_server_final();
    return ret;
}
