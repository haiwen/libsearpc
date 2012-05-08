#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib-object.h>

#include <json-glib/json-glib.h>

static const gchar *complex_object = "{\"depth1\":[1,{\"depth2\":[3,[null],\"after array\"],\"value2\":true}],\"object1\":{}}";

static const gchar *empty_object = "{\"a\":{}}";

static const gchar *reset_object = "{\"test\":\"reset\"}";
static const gchar *reset_array = "[\"reset\"]";

static void
test_builder_complex (void)
{
  JsonBuilder *builder = json_builder_new ();
  JsonNode *node;
  JsonGenerator *generator;
  gsize length;
  gchar *data;

  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "depth1");
  json_builder_begin_array (builder);
  json_builder_add_int_value (builder, 1);

  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "depth2");
  json_builder_begin_array (builder);
  json_builder_add_int_value (builder, 3);

  json_builder_begin_array (builder);
  json_builder_add_null_value (builder);
  json_builder_end_array (builder);

  json_builder_add_string_value (builder, "after array");
  json_builder_end_array (builder); /* depth2 */

  json_builder_set_member_name (builder, "value2");
  json_builder_add_boolean_value (builder, TRUE);
  json_builder_end_object (builder);

  json_builder_end_array (builder); /* depth1 */

  json_builder_set_member_name (builder, "object1");
  json_builder_begin_object (builder);
  json_builder_end_object (builder);

  json_builder_end_object (builder);

  node = json_builder_get_root (builder);
  g_object_unref (builder);

  generator = json_generator_new ();
  json_generator_set_root (generator, node);
  data = json_generator_to_data (generator, &length);

  if (g_test_verbose ())
    g_print ("Builder complex: '%*s'\n", (int)length, data);

  g_assert_cmpint (length, ==, strlen (complex_object));
  g_assert_cmpstr (data, ==, complex_object);

  g_free (data);
  json_node_free (node);
  g_object_unref (generator);
}

static void
test_builder_empty (void)
{
  JsonBuilder *builder = json_builder_new ();
  JsonNode *node;
  JsonGenerator *generator;
  gsize length;
  gchar *data;

  json_builder_begin_object (builder);

  json_builder_set_member_name (builder, "a");

  json_builder_begin_object (builder);
  json_builder_end_object (builder);

  json_builder_end_object (builder);

  node = json_builder_get_root (builder);
  g_object_unref (builder);

  generator = json_generator_new ();
  json_generator_set_root (generator, node);
  data = json_generator_to_data (generator, &length);

  if (g_test_verbose ())
    g_print ("Builder empty: '%*s'\n", (int)length, data);

  g_assert_cmpint (length, ==, strlen (empty_object));
  g_assert_cmpstr (data, ==, empty_object);

  g_free (data);
  json_node_free (node);
  g_object_unref (generator);
}

static void
test_builder_reset (void)
{
  JsonBuilder *builder = json_builder_new ();
  JsonGenerator *generator = json_generator_new ();
  JsonNode *node;
  gsize length;
  gchar *data;

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "test");
  json_builder_add_string_value (builder, "reset");
  json_builder_end_object (builder);

  node = json_builder_get_root (builder);
  json_generator_set_root (generator, node);
  data = json_generator_to_data (generator, &length);
  g_assert (strncmp (data, reset_object, length) == 0);

  g_free (data);
  json_node_free (node);

  json_builder_reset (builder);

  json_builder_begin_array (builder);
  json_builder_add_string_value (builder, "reset");
  json_builder_end_array (builder);

  node = json_builder_get_root (builder);
  json_generator_set_root (generator, node);
  data = json_generator_to_data (generator, &length);
  g_assert (strncmp (data, reset_array, length) == 0);

  g_free (data);
  json_node_free (node);
  g_object_unref (builder);
  g_object_unref (generator);
}

int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/builder/complex", test_builder_complex);
  g_test_add_func ("/builder/complex", test_builder_empty);
  g_test_add_func ("/builder/reset", test_builder_reset);

  return g_test_run ();
}
