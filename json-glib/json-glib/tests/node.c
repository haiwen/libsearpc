#include <glib.h>
#include <json-glib/json-glib.h>
#include <string.h>

static void
test_copy_null (void)
{
  JsonNode *node = json_node_new (JSON_NODE_NULL);
  JsonNode *copy = json_node_copy (node);

  g_assert_cmpint (json_node_get_node_type (node), ==, json_node_get_node_type (copy));
  g_assert_cmpint (json_node_get_value_type (node), ==, json_node_get_value_type (copy));
  g_assert_cmpstr (json_node_type_name (node), ==, json_node_type_name (copy));

  json_node_free (copy);
  json_node_free (node);
}

static void
test_copy_value (void)
{
  JsonNode *node = json_node_new (JSON_NODE_VALUE);
  JsonNode *copy;

  json_node_set_string (node, "hello");

  copy = json_node_copy (node);
  g_assert_cmpint (json_node_get_node_type (node), ==, json_node_get_node_type (copy));
  g_assert_cmpstr (json_node_type_name (node), ==, json_node_type_name (copy));
  g_assert_cmpstr (json_node_get_string (node), ==, json_node_get_string (copy));

  json_node_free (copy);
  json_node_free (node);
}

static void
test_copy_object (void)
{
  JsonObject *obj = json_object_new ();
  JsonNode *node = json_node_new (JSON_NODE_OBJECT);
  JsonNode *value = json_node_new (JSON_NODE_VALUE);
  JsonNode *copy;

  json_node_set_int (value, 42);
  json_object_set_member (obj, "answer", value);

  json_node_take_object (node, obj);

  copy = json_node_copy (node);

  g_assert_cmpint (json_node_get_node_type (node), ==, json_node_get_node_type (copy));
  g_assert (json_node_get_object (node) == json_node_get_object (copy));

  json_node_free (copy);
  json_node_free (node);
}

static void
test_null (void)
{
  JsonNode *node = json_node_new (JSON_NODE_NULL);

  g_assert (JSON_NODE_HOLDS_NULL (node));
  g_assert_cmpint (json_node_get_value_type (node), ==, G_TYPE_INVALID);
  g_assert_cmpstr (json_node_type_name (node), ==, "NULL");

  json_node_free (node);
}

static void
test_value (void)
{
  JsonNode *node = json_node_new (JSON_NODE_VALUE);
  GValue value = { 0, };
  GValue check = { 0, };

  g_assert_cmpint (JSON_NODE_TYPE (node), ==, JSON_NODE_VALUE);

  g_value_init (&value, G_TYPE_INT64);
  g_value_set_int64 (&value, 42);

  g_assert_cmpint (G_VALUE_TYPE (&value), ==, G_TYPE_INT64);
  g_assert_cmpint (g_value_get_int64 (&value), ==, 42);

  json_node_set_value (node, &value);
  json_node_get_value (node, &check);

  g_assert_cmpint (G_VALUE_TYPE (&value), ==, G_VALUE_TYPE (&check));
  g_assert_cmpint (g_value_get_int64 (&value), ==, g_value_get_int64 (&check));
  g_assert_cmpint (G_VALUE_TYPE (&check), ==, G_TYPE_INT64);
  g_assert_cmpint (g_value_get_int64 (&check), ==, 42);

  g_value_unset (&value);
  g_value_unset (&check);
  json_node_free (node);
}

int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/nodes/null-node", test_null);
  g_test_add_func ("/nodes/copy-null", test_copy_null);
  g_test_add_func ("/nodes/copy-value", test_copy_value);
  g_test_add_func ("/nodes/copy-object", test_copy_object);
  g_test_add_func ("/nodes/value", test_value);

  return g_test_run ();
}
