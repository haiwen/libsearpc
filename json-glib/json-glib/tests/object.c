#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <json-glib/json-glib.h>

static void
test_empty_object (void)
{
  JsonObject *object = json_object_new ();

  g_assert_cmpint (json_object_get_size (object), ==, 0);
  g_assert (json_object_get_members (object) == NULL);

  json_object_unref (object);
}

static void
test_add_member (void)
{
  JsonObject *object = json_object_new ();
  JsonNode *node = json_node_new (JSON_NODE_NULL);

  g_assert_cmpint (json_object_get_size (object), ==, 0);

  json_object_set_member (object, "Null", node);
  g_assert_cmpint (json_object_get_size (object), ==, 1);

  node = json_object_get_member (object, "Null");
  g_assert_cmpint (JSON_NODE_TYPE (node), ==, JSON_NODE_NULL);

  json_object_unref (object);
}

static void
test_set_member (void)
{
  JsonNode *node = json_node_new (JSON_NODE_VALUE);
  JsonObject *object = json_object_new ();

  g_assert_cmpint (json_object_get_size (object), ==, 0);

  json_node_set_string (node, "Hello");

  json_object_set_member (object, "String", node);
  g_assert_cmpint (json_object_get_size (object), ==, 1);

  node = json_object_get_member (object, "String");
  g_assert_cmpint (JSON_NODE_TYPE (node), ==, JSON_NODE_VALUE);
  g_assert_cmpstr (json_node_get_string (node), ==, "Hello");

  json_object_set_string_member (object, "String", "World");
  node = json_object_get_member (object, "String");
  g_assert_cmpint (JSON_NODE_TYPE (node), ==, JSON_NODE_VALUE);
  g_assert_cmpstr (json_node_get_string (node), ==, "World");

  json_object_set_string_member (object, "String", "Goodbye");
  g_assert_cmpstr (json_object_get_string_member (object, "String"), ==, "Goodbye");

  json_object_unref (object);
}

static void
test_remove_member (void)
{
  JsonObject *object = json_object_new ();
  JsonNode *node = json_node_new (JSON_NODE_NULL);

  json_object_set_member (object, "Null", node);

  json_object_remove_member (object, "Null");
  g_assert_cmpint (json_object_get_size (object), ==, 0);

  json_object_unref (object);
}

typedef struct _TestForeachFixture
{
  gint n_members;
} TestForeachFixture;

static const struct {
  const gchar *member_name;
  JsonNodeType member_type;
  GType member_gtype;
} type_verify[] = {
  { "integer", JSON_NODE_VALUE, G_TYPE_INT64 },
  { "boolean", JSON_NODE_VALUE, G_TYPE_BOOLEAN },
  { "string", JSON_NODE_VALUE, G_TYPE_STRING },
  { "null", JSON_NODE_NULL, G_TYPE_INVALID }
};

static void
verify_foreach (JsonObject  *object,
                const gchar *member_name,
                JsonNode    *member_node,
                gpointer     user_data)
{
  TestForeachFixture *fixture = user_data;
  gint i;

  for (i = 0; i < G_N_ELEMENTS (type_verify); i++)
    {
      if (strcmp (member_name, type_verify[i].member_name) == 0)
        {
          g_assert (json_node_get_node_type (member_node) == type_verify[i].member_type);
          g_assert (json_node_get_value_type (member_node) == type_verify[i].member_gtype);
          break;
        }
    }

  fixture->n_members += 1;
}

static void
test_foreach_member (void)
{
  JsonObject *object = json_object_new ();
  TestForeachFixture fixture = { 0, };

  json_object_set_int_member (object, "integer", 42);
  json_object_set_boolean_member (object, "boolean", TRUE);
  json_object_set_string_member (object, "string", "hello");
  json_object_set_null_member (object, "null");

  json_object_foreach_member (object, verify_foreach, &fixture);

  g_assert_cmpint (fixture.n_members, ==, json_object_get_size (object));

  json_object_unref (object);
}

static void
test_empty_member (void)
{
  JsonObject *object = json_object_new ();

  json_object_set_string_member (object, "string", "");
  g_assert (json_object_has_member (object, "string"));
  g_assert_cmpstr (json_object_get_string_member (object, "string"), ==, "");

  json_object_set_string_member (object, "null", NULL);
  g_assert (json_object_has_member (object, "null"));
  g_assert (json_object_get_string_member (object, "null") == NULL);

  json_object_unref (object);
}

int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/object/empty-object", test_empty_object);
  g_test_add_func ("/object/add-member", test_add_member);
  g_test_add_func ("/object/set-member", test_set_member);
  g_test_add_func ("/object/remove-member", test_remove_member);
  g_test_add_func ("/object/foreach-member", test_foreach_member);
  g_test_add_func ("/object/empty-member", test_empty_member);

  return g_test_run ();
}
