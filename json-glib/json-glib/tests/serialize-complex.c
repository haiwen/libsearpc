#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib-object.h>

#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

#define TEST_TYPE_BOXED                 (test_boxed_get_type ())
#define TEST_TYPE_OBJECT                (test_object_get_type ())
#define TEST_OBJECT(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEST_TYPE_OBJECT, TestObject))
#define TEST_IS_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEST_TYPE_OBJECT))
#define TEST_OBJECT_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), TEST_TYPE_OBJECT, TestObjectClass))
#define TEST_IS_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), TEST_TYPE_OBJECT))
#define TEST_OBJECT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), TEST_TYPE_OBJECT, TestObjectClass))

typedef struct _TestBoxed               TestBoxed;
typedef struct _TestObject              TestObject;
typedef struct _TestObjectClass         TestObjectClass;

struct _TestBoxed
{
  gint foo;
  gboolean bar;
};

struct _TestObject
{
  GObject parent_instance;

  gint foo;
  gboolean bar;
  gchar *baz;
  TestBoxed blah;
  gdouble meh;
};

struct _TestObjectClass
{
  GObjectClass parent_class;
};

GType test_object_get_type (void);

/*** implementation ***/

static TestBoxed *
test_boxed_copy (const TestBoxed *src)
{
  TestBoxed *copy = g_slice_new (TestBoxed);

  *copy = *src;

  return copy;
}

static void
test_boxed_free (TestBoxed *boxed)
{
  if (G_LIKELY (boxed))
    {
      g_slice_free (TestBoxed, boxed);
    }
}

GType
test_boxed_get_type (void)
{
  static GType b_type = 0;

  if (G_UNLIKELY (b_type == 0))
    b_type = g_boxed_type_register_static ("TestBoxed",
                                           (GBoxedCopyFunc) test_boxed_copy,
                                           (GBoxedFreeFunc) test_boxed_free);

  return b_type;
}

enum
{
  PROP_0,

  PROP_FOO,
  PROP_BAR,
  PROP_BAZ,
  PROP_BLAH,
  PROP_MEH
};

static JsonSerializableIface *serializable_iface = NULL;

static void json_serializable_iface_init (gpointer g_iface);

G_DEFINE_TYPE_WITH_CODE (TestObject, test_object, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (JSON_TYPE_SERIALIZABLE,
                                                json_serializable_iface_init));

static JsonNode *
test_object_serialize_property (JsonSerializable *serializable,
                                const gchar      *name,
                                const GValue     *value,
                                GParamSpec       *pspec)
{
  JsonNode *retval = NULL;

  if (strcmp (name, "blah") == 0)
    {
      TestBoxed *boxed;
      JsonObject *obj;
      JsonNode *val;

      retval = json_node_new (JSON_NODE_OBJECT);
      obj = json_object_new ();
      
      boxed = g_value_get_boxed (value);

      val = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (val, boxed->foo);
      json_object_set_member (obj, "foo", val);

      val = json_node_new (JSON_NODE_VALUE);
      json_node_set_boolean (val, boxed->bar);
      json_object_set_member (obj, "bar", val);

      json_node_take_object (retval, obj);
    }
  else
    retval = serializable_iface->serialize_property (serializable,
                                                     name,
                                                     value, pspec);

  return retval;
}

static void
json_serializable_iface_init (gpointer g_iface)
{
  JsonSerializableIface *iface = g_iface;

  serializable_iface = g_type_default_interface_peek (JSON_TYPE_SERIALIZABLE);

  iface->serialize_property = test_object_serialize_property;
}

static void
test_object_finalize (GObject *gobject)
{
  g_free (TEST_OBJECT (gobject)->baz);

  G_OBJECT_CLASS (test_object_parent_class)->finalize (gobject);
}

static void
test_object_set_property (GObject      *gobject,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  switch (prop_id)
    {
    case PROP_FOO:
      TEST_OBJECT (gobject)->foo = g_value_get_int (value);
      break;
    case PROP_BAR:
      TEST_OBJECT (gobject)->bar = g_value_get_boolean (value);
      break;
    case PROP_BAZ:
      g_free (TEST_OBJECT (gobject)->baz);
      TEST_OBJECT (gobject)->baz = g_value_dup_string (value);
      break;
    case PROP_MEH:
      TEST_OBJECT (gobject)->meh = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
test_object_get_property (GObject    *gobject,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  switch (prop_id)
    {
    case PROP_FOO:
      g_value_set_int (value, TEST_OBJECT (gobject)->foo);
      break;
    case PROP_BAR:
      g_value_set_boolean (value, TEST_OBJECT (gobject)->bar);
      break;
    case PROP_BAZ:
      g_value_set_string (value, TEST_OBJECT (gobject)->baz);
      break;
    case PROP_BLAH:
      g_value_set_boxed (value, &(TEST_OBJECT (gobject)->blah));
      break;
    case PROP_MEH:
      g_value_set_double (value, TEST_OBJECT (gobject)->meh);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
test_object_class_init (TestObjectClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = test_object_set_property;
  gobject_class->get_property = test_object_get_property;
  gobject_class->finalize = test_object_finalize;

  g_object_class_install_property (gobject_class,
                                   PROP_FOO,
                                   g_param_spec_int ("foo", "Foo", "Foo",
                                                     0, G_MAXINT, 42,
                                                     G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_BAR,
                                   g_param_spec_boolean ("bar", "Bar", "Bar",
                                                         TRUE,
                                                         G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_BAZ,
                                   g_param_spec_string ("baz", "Baz", "Baz",
                                                        NULL,
                                                        G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_BLAH,
                                   g_param_spec_boxed ("blah", "Blah", "Blah",
                                                       TEST_TYPE_BOXED,
                                                       G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_MEH,
                                   g_param_spec_double ("meh", "Meh", "Meh",
                                                        0.0, 1.0, 0.0,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
test_object_init (TestObject *object)
{
  object->foo = 42;
  object->bar = TRUE;
  object->baz = g_strdup ("Test");
  object->meh = 0.0;

  object->blah.foo = object->foo;
  object->blah.bar = object->bar;
}

static void
test_serialize (void)
{
  TestObject *obj = g_object_new (TEST_TYPE_OBJECT,
                                  "foo", 47,
                                  "bar", FALSE,
                                  "baz", "Hello, World!",
                                  "meh", 0.5,
                                  NULL);
  JsonParser *parser = json_parser_new ();
  GError *error = NULL;
  JsonObject *object;
  JsonNode *node;
  gchar *data;
  gsize len;

  data = json_gobject_to_data (G_OBJECT (obj), &len);

  g_assert_cmpint (len, >, 0);
  if (g_test_verbose ())
    g_print ("TestObject:\n%s\n", data);

  parser = json_parser_new ();
  json_parser_load_from_data (parser, data, -1, &error);
  g_assert (error == NULL);

  node = json_parser_get_root (parser);
  g_assert (json_node_get_node_type (node) == JSON_NODE_OBJECT);

  object = json_node_get_object (node);
  g_assert_cmpint (json_object_get_int_member (object, "foo"), ==, 47);
  g_assert (!json_object_get_boolean_member (object, "bar"));
  g_assert_cmpstr (json_object_get_string_member (object, "baz"), ==, "Hello, World!");
  g_assert_cmpfloat (json_object_get_double_member (object, "meh"), ==, 0.5);

  /* blah is read-only */
  g_assert (json_object_has_member (object, "blah"));

  g_free (data);
  g_object_unref (parser);
  g_object_unref (obj);
}

int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/serialize/gobject-boxed", test_serialize);

  return g_test_run ();
}
