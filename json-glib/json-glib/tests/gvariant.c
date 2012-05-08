#include <glib.h>
#include <json-glib/json-glib.h>
#include <string.h>

typedef struct
{
  gchar *test_name;
  gchar *signature;
  gchar *variant_data;
  gchar *json_data;
} TestCase;

/* each entry in this list spawns to a GVariant-to-JSON and
   JSON-to-GVariant test */
const TestCase test_cases[] =
  {
    /* boolean */
    { "/boolean", "(b)", "(true,)", "[true]" },

    /* byte */
    { "/byte", "(y)", "(byte 0xff,)", "[255]" },

    /* int16 */
    { "/int16", "(n)", "(int16 -12345,)", "[-12345]" },

    /* uint16 */
    { "/uint16", "(q)", "(uint16 40001,)", "[40001]" },

    /* int32 */
    { "/int32", "(i)", "(-7654321,)", "[-7654321]" },

    /* uint32 */
    { "/uint32", "(u)", "(uint32 12345678,)", "[12345678]" },

    /* int64 */
    { "/int64", "(x)", "(int64 -666999666999,)", "[-666999666999]" },

    /* uint64 */
    { "/uint64", "(t)", "(uint64 1999999999999999,)", "[1999999999999999]" },

    /* handle */
    { "/handle", "(h)", "(handle 1,)", "[1]" },

    /* double */
    { "/double", "(d)", "(1.23,)", "[1.23]" },

    /* string */
    { "/string", "(s)", "('hello world!',)", "[\"hello world!\"]" },

    /* object-path */
    { "/object-path", "(o)", "(objectpath '/org/gtk/json_glib',)", "[\"/org/gtk/json_glib\"]" },

    /* signature */
    { "/signature", "(g)", "(signature '(asna{sv}i)',)", "[\"(asna{sv}i)\"]" },

    /* maybe - null string */
    { "/maybe/simple/null", "(ms)", "(@ms nothing,)", "[null]" },

    /* maybe - simple string */
    { "/maybe/simple/string", "(ms)", "(@ms 'maybe string',)", "[\"maybe string\"]" },

    /* maybe - null container */
    { "/maybe/container/null", "(m(sn))", "(@m(sn) nothing,)", "[null]" },

    /* maybe - tuple container */
    { "/maybe/container/tuple", "(m(sn))", "(@m(sn) ('foo', 0),)", "[[\"foo\",0]]" },

    /* maybe - variant boolean */
    { "/maybe/variant/boolean", "(mv)", "(@mv <true>,)", "[true]" },

    /* empty array */
    { "/array/empty", "as", "@as []", "[]" },

    /* array of bytes */
    { "/array/byte", "ay", "[byte 0x01, 0x0a, 0x03, 0xff]", "[1,10,3,255]" },

    /* array of strings */
    { "/array/string", "as", "['a', 'b', 'ab', 'ba']", "[\"a\",\"b\",\"ab\",\"ba\"]" },

    /* array of array of int32 */
    { "/array/array/int32", "aai", "[[1, 2], [3, 4], [5, 6]]", "[[1,2],[3,4],[5,6]]" },

    /* array of variants */
    { "/array/variant", "av", "[<true>, <int64 1>, <'oops'>, <int64 -2>, <0.5>]", "[true,1,\"oops\",-2,0.5]" },

    /* tuple */
    { "/tuple", "(bynqiuxthds)",
      "(false, byte 0x00, int16 -1, uint16 1, -2, uint32 2, int64 429496729, uint64 3, handle 16, 2.48, 'end')",
      "[false,0,-1,1,-2,2,429496729,3,16,2.48,\"end\"]" },

    /* empty dictionary */
    { "/dictionary/empty", "a{sv}", "@a{sv} {}", "{}" },

    /* single dictionary entry */
    { "/dictionary/single-entry", "{ss}", "{'hello', 'world'}", "{\"hello\":\"world\"}" },

    /* dictionary - string : int32 */
    { "/dictionary/string-int32", "a{si}", "{'foo': 1, 'bar': 2}", "{\"foo\":1,\"bar\":2}" },

    /* dictionary - string : variant */
    { "/dictionary/string-variant", "a{sv}", "{'str': <'hi!'>, 'bool': <true>}", "{\"str\":\"hi!\",\"bool\":true}" },

    /* dictionary - int64 : variant */
    { "/dictionary/int64-variant", "a{xv}",
      "{int64 -5: <'minus five'>, 10: <'ten'>}", "{\"-5\":\"minus five\",\"10\":\"ten\"}" },

    /* dictionary - uint64 : variant */
    { "/dictionary/uint64-boolean", "a{tb}",
      "{uint64 999888777666: true, 0: false}", "{\"999888777666\":true,\"0\":false}" },

    /* dictionary - boolean : variant */
    { "/dictionary/boolean-variant", "a{by}", "{true: byte 0x01, false: 0x00}", "{\"true\":1,\"false\":0}" },

    /* dictionary - double : string */
    { "/dictionary/double-string", "a{ds}", "{1.0: 'one point zero'}", "{\"1.000000\":\"one point zero\"}" },

    /* variant - string */
    { "/variant/string", "(v)", "(<'string within variant'>,)", "[\"string within variant\"]" },

    /* variant - maybe null  */
    { "/variant/maybe/null", "(v)", "(<@mv nothing>,)", "[null]" },

    /* variant - dictionary */
    { "/variant/dictionary", "v", "<{'foo': <'bar'>, 'hi': <int64 1024>}>", "{\"foo\":\"bar\",\"hi\":1024}" },

    /* variant - variant - array */
    { "/variant/variant/array", "v", "<[<'any'>, <'thing'>, <int64 0>, <int64 -1>]>", "[\"any\",\"thing\",0,-1]" },

    /* deep-nesting */
    { "/deep-nesting",
      "a(a(a(a(a(a(a(a(a(a(s))))))))))",
      "[([([([([([([([([([('sorprise',)],)],)],)],)],)],)],)],)],)]",
      "[[[[[[[[[[[[[[[[[[[[\"sorprise\"]]]]]]]]]]]]]]]]]]]]" },

    /* mixed1 */
    { "/mixed1",
      "a{s(syba(od))}",
      "{'foo': ('bar', byte 0x64, true, [(objectpath '/baz', 1.3), ('/cat', -2.5)])}",
      "{\"foo\":[\"bar\",100,true,[[\"/baz\",1.3],[\"/cat\",-2.5]]]}" },

    /* mixed2 */
    { "/mixed2",
      "(a{by}amsvmaba{qm(sg)})",
      "({true: byte 0x01, false: 0x00}, [@ms 'do', nothing, 'did'], <@av []>, @mab nothing, {uint16 10000: @m(sg) ('yes', 'august'), 0: nothing})",
      "[{\"true\":1,\"false\":0},[\"do\",null,\"did\"],[],null,{\"10000\":[\"yes\",\"august\"],\"0\":null}]" },
  };

static void
test_gvariant_to_json (gconstpointer test_data)
{
  TestCase *test_case = (TestCase *) test_data;
  GVariant *variant;
  gchar *json_data;
  gsize len;

  variant = g_variant_parse (G_VARIANT_TYPE (test_case->signature),
                             test_case->variant_data,
                             NULL,
                             NULL,
                             NULL);

  json_data = json_gvariant_serialize_data (variant, &len);
  g_assert (json_data != NULL);

  g_assert_cmpstr (test_case->json_data, ==, json_data);

  g_free (json_data);
  g_variant_unref (variant);
}

static void
test_json_to_gvariant (gconstpointer test_data)
{
  TestCase *test_case = (TestCase *) test_data;
  GVariant *variant;
  gchar *variant_data;
  GError *error = NULL;

  variant = json_gvariant_deserialize_data (test_case->json_data,
                                            -1,
                                            test_case->signature,
                                            &error);

  if (variant == NULL)
    {
      g_assert_no_error (error);
      g_error_free (error);
    }
  else
    {
      variant_data = g_variant_print (variant, TRUE);

      g_assert_cmpstr (test_case->variant_data, ==, variant_data);

      g_free (variant_data);
      g_variant_unref (variant);
    }
}

gint
main (gint argc, gchar *argv[])
{
  gint i;
  TestCase test_case;
  gchar *test_name;

  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  /* GVariant to JSON */
  for (i = 0; i < sizeof (test_cases) / sizeof (TestCase); i++)
    {
      test_case = test_cases[i];
      test_name = g_strdup_printf ("/gvariant/to-json/%s", test_case.test_name);

      g_test_add_data_func (test_name, &test_cases[i], test_gvariant_to_json);

      g_free (test_name);
    }

  /* JSON to GVariant */
  for (i = 0; i < sizeof (test_cases) / sizeof (TestCase); i++)
    {
      test_case = test_cases[i];
      test_name = g_strdup_printf ("/gvariant/from-json/%s", test_case.test_name);

      g_test_add_data_func (test_name, &test_cases[i], test_json_to_gvariant);

      g_free (test_name);
    }

  return g_test_run ();
}
