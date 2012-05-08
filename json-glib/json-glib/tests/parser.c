#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <json-glib/json-glib.h>

static const gchar *test_empty_string = "";
static const gchar *test_empty_array_string = "[ ]";
static const gchar *test_empty_object_string = "{ }";

static void
verify_int_value (JsonNode *node)
{
  g_assert_cmpint (42, ==, json_node_get_int (node));
}

static void
verify_boolean_value (JsonNode *node)
{
  g_assert_cmpint (TRUE, ==, json_node_get_boolean (node));
}

static void
verify_string_value (JsonNode *node)
{
  g_assert_cmpstr ("string", ==, json_node_get_string (node));
}

static void
verify_double_value (JsonNode *node)
{
  g_assert_cmpfloat (10.2e3, ==, json_node_get_double (node));
}

static const struct {
  const gchar *str;
  JsonNodeType type;
  GType gtype;
  void (* verify_value) (JsonNode *node);
} test_base_values[] = {
  { "null",       JSON_NODE_NULL,  G_TYPE_INVALID, NULL, },
  { "42",         JSON_NODE_VALUE, G_TYPE_INT64,   verify_int_value },
  { "true",       JSON_NODE_VALUE, G_TYPE_BOOLEAN, verify_boolean_value },
  { "\"string\"", JSON_NODE_VALUE, G_TYPE_STRING,  verify_string_value },
  { "10.2e3",     JSON_NODE_VALUE, G_TYPE_DOUBLE,  verify_double_value }
};

static const struct {
  const gchar *str;
  gint len;
  gint element;
  JsonNodeType type;
  GType gtype;
} test_simple_arrays[] = {
  { "[ true ]",                 1, 0, JSON_NODE_VALUE, G_TYPE_BOOLEAN },
  { "[ true, false, null ]",    3, 2, JSON_NODE_NULL,  G_TYPE_INVALID },
  { "[ 1, 2, 3.14, \"test\" ]", 4, 3, JSON_NODE_VALUE, G_TYPE_STRING  }
};

static const gchar *test_nested_arrays[] = {
  "[ 42, [ ], null ]",
  "[ [ ], [ true, [ true ] ] ]",
  "[ [ false, true, 42 ], [ true, false, 3.14 ], \"test\" ]",
  "[ true, { } ]",
  "[ false, { \"test\" : 42 } ]",
  "[ { \"test\" : 42 }, null ]",
  "[ true, { \"test\" : 42 }, null ]",
  "[ { \"channel\" : \"/meta/connect\" } ]"
};

static const struct {
  const gchar *str;
  gint size;
  const gchar *member;
  JsonNodeType type;
  GType gtype;
} test_simple_objects[] = {
  { "{ \"test\" : 42 }", 1, "test", JSON_NODE_VALUE, G_TYPE_INT64 },
  { "{ \"name\" : \"\", \"state\" : 1 }", 2, "name", JSON_NODE_VALUE, G_TYPE_STRING },
  { "{ \"foo\" : \"bar\", \"baz\" : null }", 2, "baz", JSON_NODE_NULL, G_TYPE_INVALID },
  { "{ \"channel\" : \"/meta/connect\" }", 1, "channel", JSON_NODE_VALUE, G_TYPE_STRING },
  { "{ \"halign\":0.5, \"valign\":0.5 }", 2, "valign", JSON_NODE_VALUE, G_TYPE_DOUBLE }
};

static const gchar *test_nested_objects[] = {
  "{ \"array\" : [ false, \"foo\" ], \"object\" : { \"foo\" : true } }",
  "{ "
    "\"type\" : \"ClutterGroup\", "
    "\"width\" : 1, "
    "\"children\" : [ "
      "{ "
        "\"type\" : \"ClutterRectangle\", "
        "\"children\" : [ "
          "{ \"type\" : \"ClutterText\", \"text\" : \"hello there\" }"
        "] "
      "}, "
      "{ "
        "\"type\" : \"ClutterGroup\", "
        "\"width\" : 1, "
        "\"children\" : [ "
          "{ \"type\" : \"ClutterText\", \"text\" : \"hello\" }"
        "] "
      "} "
    "] "
  "}"
};

static const struct {
  const gchar *str;
  const gchar *var;
} test_assignments[] = {
  { "var foo = [ false, false, true ]", "foo" },
  { "var bar = [ true, 42 ];", "bar" },
  { "var baz = { \"foo\" : false }", "baz" }
};

static const struct
{
  const gchar *str;
  const gchar *member;
  const gchar *match;
} test_unicode[] = {
  { "{ \"test\" : \"foo \\u00e8\" }", "test", "foo è" }
};

static const struct
{
  const gchar *str;
  JsonParserError code;
} test_invalid[] = {
  { "test", JSON_PARSER_ERROR_INVALID_BAREWORD },
  { "[ foo, ]", JSON_PARSER_ERROR_INVALID_BAREWORD },
  { "[ true, ]", JSON_PARSER_ERROR_TRAILING_COMMA },
  { "{ \"foo\" : true \"bar\" : false }", JSON_PARSER_ERROR_MISSING_COMMA },
  { "[ true, [ false, ] ]", JSON_PARSER_ERROR_TRAILING_COMMA },
  { "{ \"foo\" : { \"bar\" : false, } }", JSON_PARSER_ERROR_TRAILING_COMMA },
  { "[ { }, { }, { }, ]", JSON_PARSER_ERROR_TRAILING_COMMA },
  { "{ \"foo\" false }", JSON_PARSER_ERROR_MISSING_COLON }
};

static guint n_test_base_values    = G_N_ELEMENTS (test_base_values);
static guint n_test_simple_arrays  = G_N_ELEMENTS (test_simple_arrays);
static guint n_test_nested_arrays  = G_N_ELEMENTS (test_nested_arrays);
static guint n_test_simple_objects = G_N_ELEMENTS (test_simple_objects);
static guint n_test_nested_objects = G_N_ELEMENTS (test_nested_objects);
static guint n_test_assignments    = G_N_ELEMENTS (test_assignments);
static guint n_test_unicode        = G_N_ELEMENTS (test_unicode);
static guint n_test_invalid        = G_N_ELEMENTS (test_invalid);

static void
test_empty (void)
{
  JsonParser *parser;
  GError *error = NULL;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with empty string...\n");

  if (!json_parser_load_from_data (parser, test_empty_string, -1, &error))
    {
      if (g_test_verbose ())
        g_print ("Error: %s\n", error->message);
      g_error_free (error);
      g_object_unref (parser);
      exit (1);
    }
  else
    {
      if (g_test_verbose ())
        g_print ("checking json_parser_get_root...\n");

      g_assert (NULL == json_parser_get_root (parser));
    }

  g_object_unref (parser);
}

static void
test_base_value (void)
{
  gint i;
  JsonParser *parser;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with base-values...\n");

  for (i = 0; i < n_test_base_values; i++)
    {
      GError *error = NULL;

      if (!json_parser_load_from_data (parser, test_base_values[i].str, -1, &error))
        {
          if (g_test_verbose ())
            g_print ("Error: %s\n", error->message);

          g_error_free (error);
          g_object_unref (parser);
          exit (1);
        }
      else
        {
          JsonNode *root;

          g_assert (NULL != json_parser_get_root (parser));

          root = json_parser_get_root (parser);
          g_assert (root != NULL);
          g_assert (json_node_get_parent (root) == NULL);

          if (g_test_verbose ())
            g_print ("checking root node is of the desired type %s...\n",
                     test_base_values[i].gtype == G_TYPE_INVALID ? "<null>"
                                                                 : g_type_name (test_base_values[i].gtype));
          g_assert_cmpint (JSON_NODE_TYPE (root), ==, test_base_values[i].type);
          g_assert_cmpint (json_node_get_value_type (root), ==, test_base_values[i].gtype);

          if (test_base_values[i].verify_value)
            test_base_values[i].verify_value (root);
       }
    }

  g_object_unref (parser);
}

static void
test_empty_array (void)
{
  JsonParser *parser;
  GError *error = NULL;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with empty array...\n");

  if (!json_parser_load_from_data (parser, test_empty_array_string, -1, &error))
    {
      if (g_test_verbose ())
        g_print ("Error: %s\n", error->message);
      g_error_free (error);
      g_object_unref (parser);
      exit (1);
    }
  else
    {
      JsonNode *root;
      JsonArray *array;

      g_assert (NULL != json_parser_get_root (parser));

      if (g_test_verbose ())
        g_print ("checking root node is an array...\n");
      root = json_parser_get_root (parser);
      g_assert_cmpint (JSON_NODE_TYPE (root), ==, JSON_NODE_ARRAY);
      g_assert (json_node_get_parent (root) == NULL);

      array = json_node_get_array (root);
      g_assert (array != NULL);

      if (g_test_verbose ())
        g_print ("checking array is empty...\n");
      g_assert_cmpint (json_array_get_length (array), ==, 0);
    }

  g_object_unref (parser);
}

static void
test_simple_array (void)
{
  gint i;
  JsonParser *parser;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with simple arrays...\n");

  for (i = 0; i < n_test_simple_arrays; i++)
    {
      GError *error = NULL;

      if (g_test_verbose ())
        g_print ("Parsing: '%s'\n", test_simple_arrays[i].str);

      if (!json_parser_load_from_data (parser, test_simple_arrays[i].str, -1, &error))
        {
          if (g_test_verbose ())
            g_print ("Error: %s\n", error->message);

          g_error_free (error);
          g_object_unref (parser);
          exit (1);
        }
      else
        {
          JsonNode *root, *node;
          JsonArray *array;

          g_assert (NULL != json_parser_get_root (parser));

          if (g_test_verbose ())
            g_print ("checking root node is an array...\n");
          root = json_parser_get_root (parser);
          g_assert_cmpint (JSON_NODE_TYPE (root), ==, JSON_NODE_ARRAY);
          g_assert (json_node_get_parent (root) == NULL);

          array = json_node_get_array (root);
          g_assert (array != NULL);

          if (g_test_verbose ())
            g_print ("checking array is of the desired length (%d)...\n",
                     test_simple_arrays[i].len);
          g_assert_cmpint (json_array_get_length (array), ==, test_simple_arrays[i].len);

          if (g_test_verbose ())
            g_print ("checking element %d is of the desired type %s...\n",
                     test_simple_arrays[i].element,
                     g_type_name (test_simple_arrays[i].gtype));
          node = json_array_get_element (array, test_simple_arrays[i].element);
          g_assert (node != NULL);
          g_assert (json_node_get_parent (node) == root);
          g_assert_cmpint (JSON_NODE_TYPE (node), ==, test_simple_arrays[i].type);
          g_assert_cmpint (json_node_get_value_type (node), ==, test_simple_arrays[i].gtype);
        }
    }

  g_object_unref (parser);
}

static void
test_nested_array (void)
{
  gint i;
  JsonParser *parser;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with nested arrays...\n");

  for (i = 0; i < n_test_nested_arrays; i++)
    {
      GError *error = NULL;

      if (!json_parser_load_from_data (parser, test_nested_arrays[i], -1, &error))
        {
          if (g_test_verbose ())
            g_print ("Error: %s\n", error->message);

          g_error_free (error);
          g_object_unref (parser);
          exit (1);
        }
      else
        {
          JsonNode *root;
          JsonArray *array;

          g_assert (NULL != json_parser_get_root (parser));

          if (g_test_verbose ())
            g_print ("checking root node is an array...\n");
          root = json_parser_get_root (parser);
          g_assert_cmpint (JSON_NODE_TYPE (root), ==, JSON_NODE_ARRAY);
          g_assert (json_node_get_parent (root) == NULL);

          array = json_node_get_array (root);
          g_assert (array != NULL);

          if (g_test_verbose ())
            g_print ("checking array is not empty...\n");
          g_assert_cmpint (json_array_get_length (array), >, 0);
        }
    }

  g_object_unref (parser);
}

static void
test_empty_object (void)
{
  JsonParser *parser;
  GError *error = NULL;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with empty object...\n");

  if (!json_parser_load_from_data (parser, test_empty_object_string, -1, &error))
    {
      if (g_test_verbose ())
        g_print ("Error: %s\n", error->message);
      g_error_free (error);
      g_object_unref (parser);
      exit (1);
    }
  else
    {
      JsonNode *root;
      JsonObject *object;

      g_assert (NULL != json_parser_get_root (parser));

      if (g_test_verbose ())
        g_print ("checking root node is an object...\n");
      root = json_parser_get_root (parser);
      g_assert (json_node_get_parent (root) == NULL);
      g_assert_cmpint (JSON_NODE_TYPE (root), ==, JSON_NODE_OBJECT);
      g_assert (json_node_get_parent (root) == NULL);

      object = json_node_get_object (root);
      g_assert (object != NULL);

      if (g_test_verbose ())
        g_print ("checking object is empty...\n");
      g_assert_cmpint (json_object_get_size (object), ==, 0);
    }

  g_object_unref (parser);
}

static void
test_simple_object (void)
{
  gint i;
  JsonParser *parser;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with simple objects...\n");

  for (i = 0; i < n_test_simple_objects; i++)
    {
      GError *error = NULL;

      if (!json_parser_load_from_data (parser, test_simple_objects[i].str, -1, &error))
        {
          if (g_test_verbose ())
            g_print ("Error: %s\n", error->message);

          g_error_free (error);
          g_object_unref (parser);
          exit (1);
        }
      else
        {
          JsonNode *root, *node;
          JsonObject *object;

          g_assert (NULL != json_parser_get_root (parser));

          if (g_test_verbose ())
            g_print ("checking root node is an object...\n");
          root = json_parser_get_root (parser);
          g_assert_cmpint (JSON_NODE_TYPE (root), ==, JSON_NODE_OBJECT);
          g_assert (json_node_get_parent (root) == NULL);

          object = json_node_get_object (root);
          g_assert (object != NULL);

          if (g_test_verbose ())
            g_print ("checking object is of the desired size (%d)...\n",
                     test_simple_objects[i].size);
          g_assert_cmpint (json_object_get_size (object), ==, test_simple_objects[i].size);

          if (g_test_verbose ())
            g_print ("checking member '%s' is of the desired type %s...\n",
                     test_simple_objects[i].member,
                     g_type_name (test_simple_objects[i].gtype));
          node = json_object_get_member (object, test_simple_objects[i].member);
          g_assert (node != NULL);
          g_assert (json_node_get_parent (node) == root);
          g_assert_cmpint (JSON_NODE_TYPE (node), ==, test_simple_objects[i].type);
          g_assert_cmpint (json_node_get_value_type (node), ==, test_simple_objects[i].gtype);
        }
    }

  g_object_unref (parser);
}

static void
test_nested_object (void)
{
  gint i;
  JsonParser *parser;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with nested objects...\n");

  for (i = 0; i < n_test_nested_objects; i++)
    {
      GError *error = NULL;

      if (!json_parser_load_from_data (parser, test_nested_objects[i], -1, &error))
        {
          if (g_test_verbose ())
            g_print ("Error: %s\n", error->message);

          g_error_free (error);
          g_object_unref (parser);
          exit (1);
        }
      else
        {
          JsonNode *root;
          JsonObject *object;

          g_assert (NULL != json_parser_get_root (parser));

          if (g_test_verbose ())
            g_print ("checking root node is an object...\n");
          root = json_parser_get_root (parser);
          g_assert_cmpint (JSON_NODE_TYPE (root), ==, JSON_NODE_OBJECT);
          g_assert (json_node_get_parent (root) == NULL);

          object = json_node_get_object (root);
          g_assert (object != NULL);

          if (g_test_verbose ())
            g_print ("checking object is not empty...\n");
          g_assert_cmpint (json_object_get_size (object), >, 0);
        }
    }

  g_object_unref (parser);
}

static void
test_assignment (void)
{
  gint i;
  JsonParser *parser;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with assignments...\n");

  for (i = 0; i < n_test_assignments; i++)
    {
      GError *error = NULL;

      if (!json_parser_load_from_data (parser, test_assignments[i].str, -1, &error))
        {
          if (g_test_verbose ())
            g_print ("Error: %s\n", error->message);

          g_error_free (error);
          g_object_unref (parser);
          exit (1);
        }
      else
        {
          gchar *var;

          if (g_test_verbose ())
            g_print ("checking assignment...\n");

          g_assert (json_parser_has_assignment (parser, &var) == TRUE);
          g_assert (var != NULL);
          g_assert_cmpstr (var, ==, test_assignments[i].var);
          g_assert (NULL != json_parser_get_root (parser));
        }
    }

  g_object_unref (parser);
}

static void
test_unicode_escape (void)
{
  gint i;
  JsonParser *parser;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with unicode escape...\n");

  for (i = 0; i < n_test_unicode; i++)
    {
      GError *error = NULL;

      if (!json_parser_load_from_data (parser, test_unicode[i].str, -1, &error))
        {
          if (g_test_verbose ())
            g_print ("Error: %s\n", error->message);

          g_error_free (error);
          g_object_unref (parser);
          exit (1);
        }
      else
        {
          JsonNode *root, *node;
          JsonObject *object;

          g_assert (NULL != json_parser_get_root (parser));

          if (g_test_verbose ())
            g_print ("checking root node is an object...\n");
          root = json_parser_get_root (parser);
          g_assert_cmpint (JSON_NODE_TYPE (root), ==, JSON_NODE_OBJECT);

          object = json_node_get_object (root);
          g_assert (object != NULL);

          if (g_test_verbose ())
            g_print ("checking object is not empty...\n");
          g_assert_cmpint (json_object_get_size (object), >, 0);

          node = json_object_get_member (object, test_unicode[i].member);
          g_assert_cmpint (JSON_NODE_TYPE (node), ==, JSON_NODE_VALUE);

          if (g_test_verbose ())
            g_print ("checking simple string equality...\n");
          g_assert_cmpstr (json_node_get_string (node), ==, test_unicode[i].match);

          if (g_test_verbose ())
            g_print ("checking for valid UTF-8...\n");
          g_assert (g_utf8_validate (json_node_get_string (node), -1, NULL));
        }
    }

  g_object_unref (parser);
}

static void
test_invalid_json (void)
{
  JsonParser *parser;
  GError *error = NULL;
  gint i;

  parser = json_parser_new ();
  g_assert (JSON_IS_PARSER (parser));

  if (g_test_verbose ())
    g_print ("checking json_parser_load_from_data with invalid data...\n");

  for (i = 0; i < n_test_invalid; i++)
    {
      gboolean res;

      if (g_test_verbose ())
        g_print ("Parsing: '%s'\n", test_invalid[i].str);

      res = json_parser_load_from_data (parser, test_invalid[i].str, -1,
                                        &error);

      g_assert (!res);
      g_assert_error (error, JSON_PARSER_ERROR, test_invalid[i].code);

      if (g_test_verbose ())
        g_print ("Error: %s\n", error->message);

      g_clear_error (&error);
    }

  g_object_unref (parser);
}

static void
test_stream_sync (void)
{
  JsonParser *parser;
  GFile *file;
  GFileInputStream *stream;
  GError *error = NULL;
  JsonNode *root;

  parser = json_parser_new ();

  file = g_file_new_for_path (TESTS_DATA_DIR "/stream-load.json");
  stream = g_file_read (file, NULL, &error);
  g_assert (error == NULL);
  g_assert (stream != NULL);

  json_parser_load_from_stream (parser, G_INPUT_STREAM (stream), NULL, &error);
  g_assert (error == NULL);

  root = json_parser_get_root (parser);
  g_assert (root != NULL);
  g_assert (JSON_NODE_HOLDS_ARRAY (root));

  g_object_unref (stream);
  g_object_unref (file);
  g_object_unref (parser);
}

static void
on_load_complete (GObject      *gobject,
                  GAsyncResult *result,
                  gpointer      user_data)
{
  JsonParser *parser = JSON_PARSER (gobject);
  GMainLoop *main_loop = user_data;
  GError *error = NULL;
  JsonNode *root;
  gboolean res;

  res = json_parser_load_from_stream_finish (parser, result, &error);
  g_assert (res);
  g_assert (error == NULL);

  root = json_parser_get_root (parser);
  g_assert (root != NULL);
  g_assert (JSON_NODE_HOLDS_ARRAY (root));

  g_main_loop_quit (main_loop);
}

static void
test_stream_async (void)
{
  GMainLoop *main_loop;
  GError *error = NULL;
  JsonParser *parser = json_parser_new ();
  GFile *file = g_file_new_for_path (TESTS_DATA_DIR "/stream-load.json");
  GFileInputStream *stream = g_file_read (file, NULL, &error);

  g_assert (error == NULL);
  g_assert (stream != NULL);

  main_loop = g_main_loop_new (NULL, FALSE);

  json_parser_load_from_stream_async (parser, G_INPUT_STREAM (stream), NULL,
                                      on_load_complete,
                                      main_loop);

  g_main_loop_run (main_loop);

  g_main_loop_unref (main_loop);
  g_object_unref (stream);
  g_object_unref (file);
  g_object_unref (parser);
}

int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/parser/empty-string", test_empty);
  g_test_add_func ("/parser/base-value", test_base_value);
  g_test_add_func ("/parser/empty-array", test_empty_array);
  g_test_add_func ("/parser/simple-array", test_simple_array);
  g_test_add_func ("/parser/nested-array", test_nested_array);
  g_test_add_func ("/parser/empty-object", test_empty_object);
  g_test_add_func ("/parser/simple-object", test_simple_object);
  g_test_add_func ("/parser/nested-object", test_nested_object);
  g_test_add_func ("/parser/assignment", test_assignment);
  g_test_add_func ("/parser/unicode-escape", test_unicode_escape);
  g_test_add_func ("/parser/invalid-json", test_invalid_json);
  g_test_add_func ("/parser/stream-sync", test_stream_sync);
  g_test_add_func ("/parser/stream-async", test_stream_async);

  return g_test_run ();
}
