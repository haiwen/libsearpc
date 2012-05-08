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

  TestBoxed blah;
};

struct _TestObjectClass
{
  GObjectClass parent_class;
};

GType test_object_get_type (void);

/*** implementation ***/

static gpointer
test_boxed_copy (gpointer src)
{
  return g_slice_dup (TestBoxed, src);
}

static void
test_boxed_free (gpointer boxed)
{
  if (G_LIKELY (boxed != NULL))
    g_slice_free (TestBoxed, boxed);
}

static JsonNode *
test_boxed_serialize (gconstpointer boxed)
{
  const TestBoxed *test = boxed;
  JsonObject *object;
  JsonNode *node;

  if (boxed == NULL)
    return json_node_new (JSON_NODE_NULL);

  object = json_object_new ();
  node = json_node_new (JSON_NODE_OBJECT);

  json_object_set_int_member (object, "foo", test->foo);
  json_object_set_boolean_member (object, "bar", test->bar);

  json_node_take_object (node, object);

  if (g_test_verbose ())
    {
      g_print ("Serialize: { foo: %" G_GINT64_FORMAT ", bar: %s }\n",
               json_object_get_int_member (object, "foo"),
               json_object_get_boolean_member (object, "bar") ? "true" : "false");
    }

  return node;
}

static gpointer
test_boxed_deserialize (JsonNode *node)
{
  JsonObject *object;
  TestBoxed *test;

  if (json_node_get_node_type (node) != JSON_NODE_OBJECT)
    return NULL;

  object = json_node_get_object (node);

  test = g_slice_new (TestBoxed);
  test->foo = json_object_get_int_member (object, "foo");
  test->bar = json_object_get_boolean_member (object, "bar");

  if (g_test_verbose ())
    {
      g_print ("Deserialize: { foo: %d, bar: %s }\n",
               test->foo,
               test->bar ? "true" : "false");
    }

  return test;
}

GType
test_boxed_get_type (void)
{
  static GType b_type = 0;

  if (G_UNLIKELY (b_type == 0))
    {
      b_type = g_boxed_type_register_static ("TestBoxed",
                                             test_boxed_copy,
                                             test_boxed_free);

      if (g_test_verbose ())
        g_print ("Registering transform functions\n");

      json_boxed_register_serialize_func (b_type, JSON_NODE_OBJECT,
                                          test_boxed_serialize);
      json_boxed_register_deserialize_func (b_type, JSON_NODE_OBJECT,
                                            test_boxed_deserialize);
    }

  return b_type;
}

enum
{
  PROP_0,

  PROP_BLAH
};

G_DEFINE_TYPE (TestObject, test_object, G_TYPE_OBJECT);

static void
test_object_finalize (GObject *gobject)
{
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
    case PROP_BLAH:
      {
        const TestBoxed *blah = g_value_get_boxed (value);

        TEST_OBJECT (gobject)->blah = *blah;
      }
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
    case PROP_BLAH:
      g_value_set_boxed (value, &(TEST_OBJECT (gobject)->blah));
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
                                   PROP_BLAH,
                                   g_param_spec_boxed ("blah", "Blah", "Blah",
                                                       TEST_TYPE_BOXED,
                                                       G_PARAM_READWRITE));
}

static void
test_object_init (TestObject *object)
{
  object->blah.foo = 0;
  object->blah.bar = FALSE;
}

static const gchar *serialize_data =
"{\n"
"  \"blah\" : {\n"
"    \"foo\" : 42,\n"
"    \"bar\" : true\n"
"  }\n"
"}";

static void
test_serialize_boxed (void)
{
  TestBoxed boxed = { 42, TRUE };
  GObject *obj;
  gchar *data;
  gsize len;

  obj = g_object_new (TEST_TYPE_OBJECT, "blah", &boxed, NULL);

  data = json_gobject_to_data (obj, &len);

  g_assert_cmpint (len, ==, strlen (serialize_data));
  g_assert_cmpstr (data, ==, serialize_data);

  if (g_test_verbose ())
    g_print ("TestObject:\n%s\n", data);

  g_free (data);
  g_object_unref (obj);
}

static void
test_deserialize_boxed (void)
{

  GObject *obj;

  obj = json_gobject_from_data (TEST_TYPE_OBJECT, serialize_data, -1, NULL);
  g_assert (TEST_IS_OBJECT (obj));
  g_assert_cmpint (TEST_OBJECT (obj)->blah.foo, ==, 42);
  g_assert (TEST_OBJECT (obj)->blah.bar);

  g_object_unref (obj);
}

int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/boxed/serialize-property", test_serialize_boxed);
  g_test_add_func ("/boxed/deserialize-property", test_deserialize_boxed);

  return g_test_run ();
}
