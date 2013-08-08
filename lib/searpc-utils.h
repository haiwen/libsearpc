#include <glib.h>
#include <glib-object.h>
#include <jansson.h>

#define SEAFILE_JSON_DOMAIN g_quark_from_string("SEAFILE_JSON")

typedef enum {
  SEAFILE_JSON_ERROR_LOAD,
  SEAFILE_JSON_ERROR_PACK,
  SEAFILE_JSON_ERROR_UPACK
}SEAFILEJSONERROR;

json_t *json_serialize_pspec (const GValue *);
json_t *json_gobject_serialize (GObject *);
gboolean json_deserialize_pspec (GValue *, GParamSpec *, json_t *);
GObject *json_gobject_deserialize (GType , json_t *);

inline static void setjetoge(const json_error_t *jerror, GError *error)
{
/* Load is the only function I use which reports errors */
  g_set_error(&error, SEAFILE_JSON_DOMAIN, SEAFILE_JSON_ERROR_LOAD, "%s", jerror->text);
}

inline static const char *json_object_get_string_or_null_member (json_t *object,const char *member_name)
{
  json_t *ret = json_object_get (object, member_name);
  if (ret)
    return json_string_value(ret);
  else
    return NULL;
}

inline static void json_object_set_string_or_null_member (json_t *object,const char *member_name,const char *value)
{
  if (value)
    json_object_set_new(object,member_name,json_string(value));
  else
    json_object_set_new(object,member_name,json_null());
}

inline static const char *json_array_get_string_or_null_element (json_t *array, size_t index)
{
  json_t *ret=json_array_get(array,index);
  if (ret)
    return json_string_value(ret);
  else
    return NULL;
}

inline static void json_array_add_string_or_null_element (json_t *array, const char *value)
{
  if (value)
    json_array_append_new(array, json_string(value));
  else
    json_array_append_new(array, json_null());
}

inline static json_int_t json_array_get_int_element (json_t *array, size_t index)
{
    return json_integer_value(json_array_get(array, index));
}

