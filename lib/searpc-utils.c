#include <errno.h>
#include <glib.h>
#include <glib-object.h>
#include <jansson.h>

#include "searpc-utils.h"

#if !GLIB_CHECK_VERSION(2, 32, 0)
#define g_value_set_schar g_value_set_char
#define g_value_get_schar g_value_get_char
#endif

static json_t *json_serialize_pspec (const GValue *value)
{
    /* Only types in json-glib but G_TYPE_BOXED */
    switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value))) {
        case G_TYPE_STRING:
            if (!g_value_get_string (value))
        break;
            else
            return json_string (g_value_get_string (value));
        case G_TYPE_BOOLEAN:
            if (g_value_get_boolean (value))
                return json_true ();
            else return json_false ();
        case G_TYPE_INT:
            return json_integer (g_value_get_int (value));
        case G_TYPE_UINT:
            return json_integer (g_value_get_uint (value));
        case G_TYPE_LONG:
            return json_integer (g_value_get_long (value));
        case G_TYPE_ULONG:
            return json_integer (g_value_get_ulong (value));
        case G_TYPE_INT64:
            return json_integer (g_value_get_int64 (value));
        case G_TYPE_FLOAT:
            return json_real (g_value_get_float (value));
        case G_TYPE_DOUBLE:
            return json_real (g_value_get_double (value));
        case G_TYPE_CHAR:
            return json_integer (g_value_get_schar (value));
        case G_TYPE_UCHAR:
            return json_integer (g_value_get_uchar (value));
        case G_TYPE_ENUM:
            return json_integer (g_value_get_enum (value));
        case G_TYPE_FLAGS:
            return json_integer (g_value_get_flags (value));
        case G_TYPE_NONE:
            break;
        case G_TYPE_OBJECT:
            {
            GObject *object = g_value_get_object (value);
            if (object)
                return json_gobject_serialize (object);
            }
            break;
        default:
            g_warning("Unsuppoted type `%s'",g_type_name (G_VALUE_TYPE (value)));
    }
    return json_null();
}

json_t *json_gobject_serialize (GObject *gobject)
{
    json_t *object = json_object();
    GParamSpec **pspecs;
    guint n_pspecs, i;

    pspecs = g_object_class_list_properties (G_OBJECT_GET_CLASS (gobject), &n_pspecs);

    for (i=0; i!=n_pspecs; ++i) {
        json_t *node;
        GParamSpec *pspec = pspecs[i];
        GValue value = { 0, };

        g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));
        g_object_get_property(gobject, pspec->name, &value);
        node=json_serialize_pspec (&value);

        if (node)
            json_object_set_new (object, pspec->name, node);

        g_value_unset (&value);
    }

    g_free (pspecs);

    return object;

}

static gboolean json_deserialize_pspec (GValue *value, GParamSpec *pspec, json_t *node)
{
    switch (json_typeof(node)) {
        case JSON_OBJECT:
            if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_OBJECT)) {
                GObject *object;
                object = json_gobject_deserialize (G_VALUE_TYPE (value), node);
                if (object)
                  g_value_take_object (value, object);
                else
                  g_value_set_object (value, NULL);

                return TRUE;
            }
            break;
      case JSON_STRING:
          if (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)) == G_TYPE_STRING) {
              g_value_set_string(value, json_string_value(node));
              return TRUE;
          }
          break;
      case JSON_INTEGER:
          {
              json_int_t int_value = json_integer_value (node);
              switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value))) {
                  case G_TYPE_CHAR:
                      g_value_set_schar(value, (gchar)int_value);
                      return TRUE;
                  case G_TYPE_UCHAR:
                      g_value_set_uchar (value, (guchar)int_value);
                      return TRUE;
                  case G_TYPE_INT:
                      g_value_set_int (value, (gint)int_value);
                      return TRUE;
                  case G_TYPE_UINT:
                      g_value_set_uint(value, (guint)int_value);
                      return TRUE;
                  case G_TYPE_LONG:
                      g_value_set_long(value, (glong)int_value);
                      return TRUE;
                  case G_TYPE_ULONG:
                      g_value_set_ulong(value, (gulong)int_value);
                      return TRUE;
                  case G_TYPE_INT64:
                      g_value_set_int64(value,(gint64)int_value);
                      return TRUE;
                  case G_TYPE_ENUM:
                      g_value_set_enum(value,(gint64)int_value);
                      return TRUE;
                  case G_TYPE_FLAGS:
                      g_value_set_flags(value,(gint64)int_value);
                      return TRUE;
              }
          }
          break;
      case JSON_REAL:
          {
              double real_value = json_real_value(node);
              switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value))) {
                  case G_TYPE_FLOAT:
                    g_value_set_float(value,(gfloat)real_value);
                    return TRUE;
                  case G_TYPE_DOUBLE:
                    g_value_set_double(value,(gdouble)real_value);
                    return TRUE;
              }
          }
        break;
      case JSON_TRUE:
      case JSON_FALSE:
          if (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)) == G_TYPE_BOOLEAN) {
              g_value_set_boolean(value,(gboolean)json_is_true(node));
              return TRUE;
          }
          break;
      case JSON_NULL:
          if (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)) == G_TYPE_STRING) {
              g_value_set_string (value, NULL);
              return TRUE;
          }
          else if (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)) == G_TYPE_OBJECT) {
              g_value_set_object (value, NULL);
              return TRUE;
          }
          break;
      case JSON_ARRAY:
          return FALSE;
          break;
      default:
          return FALSE;
  }

  return FALSE;

}

GObject *json_gobject_deserialize (GType gtype, json_t *object)
{
    GObjectClass *klass;
    GObject *ret;
    guint n_members, i;
    json_t *head, *member;
    const char *member_name;
    GArray *construct_params;

    klass = g_type_class_ref (gtype);
    n_members = json_object_size (object);
    construct_params = g_array_sized_new (FALSE, FALSE, sizeof (GParameter), n_members);
    head = json_object_iter (object);

    for (member=head; member; member=json_object_iter_next (object, member)) {
        GParamSpec *pspec;
        GParameter param = { NULL, };
        const char *member_name = json_object_iter_key (member);
        json_t *val = json_object_iter_value(member);

        pspec = g_object_class_find_property (klass, member_name);

        if (!pspec)
            continue;

        if (pspec->flags & G_PARAM_CONSTRUCT_ONLY)
            continue;

        if (!(pspec->flags & G_PARAM_WRITABLE))
            continue;

        g_value_init(&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));

        if (json_deserialize_pspec (&param.value, pspec, val)) {
            param.name = g_strdup (pspec->name);
            g_array_append_val (construct_params, param);
        }
        else
            g_warning ("Failed to deserialize \"%s\" property of type \"%s\" for an object of type \"%s\"",
                       pspec->name, g_type_name (G_VALUE_TYPE (&param.value)), g_type_name (gtype));
    }

    ret = g_object_newv (gtype, construct_params->len, (GParameter *) construct_params->data);

    for (i=0; i!= construct_params->len; ++i) {
        GParameter *param = &g_array_index (construct_params, GParameter, i);
        g_free ((gchar *) param->name);
        g_value_unset (&param->value);
    }

    g_array_free(construct_params, TRUE);
    g_type_class_unref(klass);

    return ret;

}
