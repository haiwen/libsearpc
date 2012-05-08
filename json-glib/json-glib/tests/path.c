#include <string.h>
#include <glib.h>
#include <json-glib/json-glib.h>

static const char *test_json =
"{ \"store\": {"
"    \"book\": [ "
"      { \"category\": \"reference\","
"        \"author\": \"Nigel Rees\","
"        \"title\": \"Sayings of the Century\","
"        \"price\": \"8.95\""
"      },"
"      { \"category\": \"fiction\","
"        \"author\": \"Evelyn Waugh\","
"        \"title\": \"Sword of Honour\","
"        \"price\": \"12.99\""
"      },"
"      { \"category\": \"fiction\","
"        \"author\": \"Herman Melville\","
"        \"title\": \"Moby Dick\","
"        \"isbn\": \"0-553-21311-3\","
"        \"price\": \"8.99\""
"      },"
"      { \"category\": \"fiction\","
"        \"author\": \"J. R. R. Tolkien\","
"        \"title\": \"The Lord of the Rings\","
"        \"isbn\": \"0-395-19395-8\","
"        \"price\": \"22.99\""
"      }"
"    ],"
"    \"bicycle\": {"
"      \"color\": \"red\","
"      \"price\": \"19.95\""
"    }"
"  }"
"}";

static const struct {
  const char *exp;
  const char *res;
} test_expressions[] = {
  {
    "$.store.book[0].title",
    "[\"Sayings of the Century\"]"
  },
  {
    "$['store']['book'][0]['title']",
    "[\"Sayings of the Century\"]"
  },
  {
    "$.store.book[*].author",
    "[\"Nigel Rees\",\"Evelyn Waugh\",\"Herman Melville\",\"J. R. R. Tolkien\"]"
  },
  {
    "$..author",
    "[\"Nigel Rees\",\"Evelyn Waugh\",\"Herman Melville\",\"J. R. R. Tolkien\"]"
  },
  {
    "$.store.*",
    NULL
  },
  {
    "$.store..price",
    "[\"8.95\",\"12.99\",\"8.99\",\"22.99\",\"19.95\"]"
  },
  {
    "$..book[2]",
    "[{\"category\":\"fiction\",\"author\":\"Herman Melville\",\"title\":\"Moby Dick\",\"isbn\":\"0-553-21311-3\",\"price\":\"8.99\"}]"
  },
  {
    "$..book[-1:]",
    "[{\"category\":\"fiction\",\"author\":\"J. R. R. Tolkien\",\"title\":\"The Lord of the Rings\",\"isbn\":\"0-395-19395-8\",\"price\":\"22.99\"}]"
  },
  {
    "$..book[0,1]",
    "[{\"category\":\"reference\",\"author\":\"Nigel Rees\",\"title\":\"Sayings of the Century\",\"price\":\"8.95\"},{\"category\":\"fiction\",\"author\":\"Evelyn Waugh\",\"title\":\"Sword of Honour\",\"price\":\"12.99\"}]"
  },
  {
    "$..book[:2]",
    "[{\"category\":\"reference\",\"author\":\"Nigel Rees\",\"title\":\"Sayings of the Century\",\"price\":\"8.95\"},{\"category\":\"fiction\",\"author\":\"Evelyn Waugh\",\"title\":\"Sword of Honour\",\"price\":\"12.99\"}]"
  },
};

static void
test_expression (void)
{
  JsonPath *path = json_path_new ();
  int i;

  for (i = 0; i < G_N_ELEMENTS (test_expressions); i++)
    {
      const char *expr = test_expressions[i].exp;
      GError *error = NULL;

      g_assert (json_path_compile (path, expr, &error));
      g_assert_no_error (error);
    }

  g_object_unref (path);
}

static void
test_match (void)
{
  JsonParser *parser = json_parser_new ();
  JsonGenerator *gen = json_generator_new ();
  JsonPath *path = json_path_new ();
  JsonNode *root;
  int i;

  json_parser_load_from_data (parser, test_json, -1, NULL);
  root = json_parser_get_root (parser);

  for (i = 0; i < G_N_ELEMENTS (test_expressions); i++)
    {
      const char *expr = test_expressions[i].exp;
      const char *res  = test_expressions[i].res;
      JsonNode *matches;
      char *str;

      if (res == NULL || *res == '\0')
        continue;

      g_assert (json_path_compile (path, expr, NULL));

      matches = json_path_match (path, root);
      g_assert (JSON_NODE_HOLDS_ARRAY (matches));

      json_generator_set_root (gen, matches);
      str = json_generator_to_data (gen, NULL);

      if (g_test_verbose ())
        {
          g_print ("* expr[%02d]: '%s' =>\n"
                   "- result:   %s\n"
                   "- expected: %s\n",
                   i, expr, str, res);
        }

      g_assert_cmpstr (str, ==, res);

      g_free (str);
      json_node_free (matches);
    }

  g_object_unref (parser);
  g_object_unref (path);
  g_object_unref (gen);
}

int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("http://bugzilla.gnome.org/show_bug.cgi?id=");

  g_test_add_func ("/path/expressions", test_expression);
  g_test_add_func ("/path/match", test_match);

  return g_test_run ();
}
