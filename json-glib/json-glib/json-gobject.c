/* json-gobject.c - JSON GObject integration
 * 
 * This file is part of JSON-GLib
 * Copyright (C) 2007  OpenedHand Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Author:
 *   Emmanuele Bassi  <ebassi@openedhand.com>
 */

/**
 * SECTION:json-gobject
 * @short_description: Serialize and deserialize GObjects
 *
 * JSON-GLib provides API for serializing and deserializing #GObject<!-- -->s
 * to and from JSON data streams.
 *
 * Simple #GObject classes can be (de)serialized into JSON objects, if the
 * properties have compatible types with the native JSON types (integers,
 * booleans, strings, string vectors). If the class to be (de)serialized has
 * complex data types for properties (like boxed types or other objects)
 * then the class should implement the provided #JsonSerializable interface
 * and its virtual functions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <glib/gi18n-lib.h>

#include "json-types-private.h"
#include "json-gobject-private.h"

#include "json-debug.h"
#include "json-parser.h"
#include "json-generator.h"

static gboolean
enum_from_string (GType        type,
                  const gchar *string,
                  gint        *enum_value)
{
  GEnumClass *eclass;
  GEnumValue *ev;
  gchar *endptr;
  gint value;
  gboolean retval = TRUE;

  g_return_val_if_fail (G_TYPE_IS_ENUM (type), 0);
  g_return_val_if_fail (string != NULL, 0);

  value = strtoul (string, &endptr, 0);
  if (endptr != string) /* parsed a number */
    *enum_value = value;
  else
    {
      eclass = g_type_class_ref (type);
      ev = g_enum_get_value_by_name (eclass, string);
      if (!ev)
	ev = g_enum_get_value_by_nick (eclass, string);

      if (ev)
	*enum_value = ev->value;
      else
        retval = FALSE;

      g_type_class_unref (eclass);
    }

  return retval;
}

static gboolean
flags_from_string (GType        type,
                   const gchar *string,
                   gint        *flags_value)
{
  GFlagsClass *fclass;
  gchar *endptr, *prevptr;
  guint i, j, ret, value;
  gchar *flagstr;
  GFlagsValue *fv;
  const gchar *flag;
  gunichar ch;
  gboolean eos;

  g_return_val_if_fail (G_TYPE_IS_FLAGS (type), 0);
  g_return_val_if_fail (string != 0, 0);

  ret = TRUE;

  value = strtoul (string, &endptr, 0);
  if (endptr != string) /* parsed a number */
    *flags_value = value;
  else
    {
      fclass = g_type_class_ref (type);

      flagstr = g_strdup (string);
      for (value = i = j = 0; ; i++)
	{
	  eos = flagstr[i] == '\0';

	  if (!eos && flagstr[i] != '|')
	    continue;

	  flag = &flagstr[j];
	  endptr = &flagstr[i];

	  if (!eos)
	    {
	      flagstr[i++] = '\0';
	      j = i;
	    }

	  /* trim spaces */
	  for (;;)
	    {
	      ch = g_utf8_get_char (flag);
	      if (!g_unichar_isspace (ch))
		break;
	      flag = g_utf8_next_char (flag);
	    }

	  while (endptr > flag)
	    {
	      prevptr = g_utf8_prev_char (endptr);
	      ch = g_utf8_get_char (prevptr);
	      if (!g_unichar_isspace (ch))
		break;
	      endptr = prevptr;
	    }

	  if (endptr > flag)
	    {
	      *endptr = '\0';
	      fv = g_flags_get_value_by_name (fclass, flag);

	      if (!fv)
		fv = g_flags_get_value_by_nick (fclass, flag);

	      if (fv)
		value |= fv->value;
	      else
		{
		  ret = FALSE;
		  break;
		}
	    }

	  if (eos)
	    {
	      *flags_value = value;
	      break;
	    }
	}

      g_free (flagstr);

      g_type_class_unref (fclass);
    }

  return ret;
}

static GObject *
json_gobject_new (GType       gtype,
                  JsonObject *object)
{
  JsonSerializableIface *iface = NULL;
  JsonSerializable *serializable = NULL;
  gboolean find_property;
  gboolean deserialize_property;
  gboolean set_property;
  GList *members, *members_left, *l;
  guint n_members;
  GObjectClass *klass;
  GObject *retval;
  GArray *construct_params;
  gint i;

  klass = g_type_class_ref (gtype);

  n_members = json_object_get_size (object);
  members = json_object_get_members (object);
  members_left = NULL;

  /* first pass: construct-only properties; here we cannot use Serializable
   * because we don't have an instance yet; we use the default implementation
   * of json_deserialize_pspec() to deserialize known types
   *
   * FIXME - find a way to allow deserialization for these properties
   */
  construct_params = g_array_sized_new (FALSE, FALSE, sizeof (GParameter), n_members);

  for (l = members; l != NULL; l = l->next)
    {
      const gchar *member_name = l->data;
      GParamSpec *pspec;
      GParameter param = { NULL, };
      JsonNode *val;
      gboolean res = FALSE;

      pspec = g_object_class_find_property (klass, member_name);
      if (!pspec)
        goto next_member;

      /* we only apply construct-only properties here */
      if ((pspec->flags & G_PARAM_CONSTRUCT_ONLY) == 0)
        goto next_member;

      if (!(pspec->flags & G_PARAM_WRITABLE))
        goto next_member;

      g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));

      val = json_object_get_member (object, member_name);
      res = json_deserialize_pspec (&param.value, pspec, val);
      if (!res)
	{
	  g_warning ("Failed to deserialize \"%s\" property of type \"%s\" for an object of type \"%s\"",
		     pspec->name, G_VALUE_TYPE_NAME (&param.value), g_type_name (gtype));

	  g_value_unset (&param.value);
	}
      else
        {
          param.name = g_strdup (pspec->name);

          g_array_append_val (construct_params, param);

          continue;
        }

    next_member:
      members_left = g_list_prepend (members_left, l->data);
    }

  retval = g_object_newv (gtype,
                          construct_params->len,
                          (GParameter *) construct_params->data);

  /* free the contents of the GArray */
  for (i = 0; i < construct_params->len; i++)
    {
      GParameter *param = &g_array_index (construct_params, GParameter, i);

      g_free ((gchar *) param->name);
      g_value_unset (&param->value);
    }

  g_array_free (construct_params, TRUE);
  g_list_free (members);

  /* we use g_list_prepend() above, but we want to maintain
   * the ordering of json_object_get_members() here
   */
  members = g_list_reverse (members_left);

  /* do the Serializable type check once */
  if (g_type_is_a (gtype, JSON_TYPE_SERIALIZABLE))
    {
      serializable = JSON_SERIALIZABLE (retval);
      iface = JSON_SERIALIZABLE_GET_IFACE (serializable);
      find_property = (iface->find_property != NULL);
      deserialize_property = (iface->deserialize_property != NULL);
      set_property = (iface->set_property != NULL);
    }
  else
    {
      find_property = FALSE;
      deserialize_property = FALSE;
      set_property = FALSE;
    }

  g_object_freeze_notify (retval);

  for (l = members; l != NULL; l = l->next)
    {
      const gchar *member_name = l->data;
      GParamSpec *pspec;
      JsonNode *val;
      GValue value = { 0, };
      gboolean res = FALSE;

      if (find_property)
        pspec = json_serializable_find_property (serializable, member_name);
      else
        pspec = g_object_class_find_property (klass, member_name);

      if (pspec == NULL)
        continue;

      /* we should have dealt with these above */
      if (pspec->flags & G_PARAM_CONSTRUCT_ONLY)
        continue;

      if (!(pspec->flags & G_PARAM_WRITABLE))
        continue;

      g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));

      val = json_object_get_member (object, member_name);

      if (deserialize_property)
        {
          JSON_NOTE (GOBJECT, "Using JsonSerializable for property '%s'", pspec->name);
          res = iface->deserialize_property (serializable, pspec->name,
                                             &value,
                                             pspec,
                                             val);
        }

      if (!res)
        {
          JSON_NOTE (GOBJECT, "Using json_deserialize_pspec for property '%s'", pspec->name);
          res = json_deserialize_pspec (&value, pspec, val);
        }

      if (res)
        {
          JSON_NOTE (GOBJECT, "Calling set_property('%s', '%s')",
                     pspec->name,
                     g_type_name (G_VALUE_TYPE (&value)));

          if (set_property)
            json_serializable_set_property (serializable, pspec, &value);
          else
            g_object_set_property (retval, pspec->name, &value);
        }
      else
	g_warning ("Failed to deserialize \"%s\" property of type \"%s\" for an object of type \"%s\"",
		   pspec->name, g_type_name (G_VALUE_TYPE (&value)), g_type_name (gtype));

      g_value_unset (&value);
    }

  g_list_free (members);

  g_object_thaw_notify (retval);

  g_type_class_unref (klass);

  return retval;
}

static JsonObject *
json_gobject_dump (GObject *gobject)
{
  JsonSerializableIface *iface = NULL;
  JsonSerializable *serializable = NULL;
  gboolean list_properties = FALSE;
  gboolean serialize_property = FALSE;
  gboolean get_property = FALSE;
  JsonObject *object;
  GParamSpec **pspecs;
  guint n_pspecs, i;

  if (JSON_IS_SERIALIZABLE (gobject))
    {
      serializable = JSON_SERIALIZABLE (gobject);
      iface = JSON_SERIALIZABLE_GET_IFACE (gobject);
      list_properties = (iface->list_properties != NULL);
      serialize_property = (iface->serialize_property != NULL);
      get_property = (iface->get_property != NULL);
    }

  object = json_object_new ();

  if (list_properties)
    pspecs = json_serializable_list_properties (serializable, &n_pspecs);
  else
    pspecs = g_object_class_list_properties (G_OBJECT_GET_CLASS (gobject), &n_pspecs);

  for (i = 0; i < n_pspecs; i++)
    {
      GParamSpec *pspec = pspecs[i];
      GValue value = { 0, };
      JsonNode *node = NULL;

      /* read only what we can */
      if (!(pspec->flags & G_PARAM_READABLE))
        continue;

      g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));

      if (get_property)
        json_serializable_get_property (serializable, pspec, &value);
      else
        g_object_get_property (gobject, pspec->name, &value);

      /* if there is a serialization vfunc, then it is completely responsible
       * for serializing the property, possibly by calling the implementation
       * of the default JsonSerializable interface through chaining up
       */
      if (serialize_property)
        {
          node = iface->serialize_property (serializable, pspec->name,
                                            &value,
                                            pspec);
        }
      /* skip if the value is the default for the property */
      /* else if (!g_param_value_defaults (pspec, &value)) */
      else
          node = json_serialize_pspec (&value, pspec);

      if (node)
        json_object_set_member (object, pspec->name, node);

      g_value_unset (&value);
    }

  g_free (pspecs);

  return object;
}

gboolean
json_deserialize_pspec (GValue     *value,
                        GParamSpec *pspec,
                        JsonNode   *node)
{
  GValue node_value = { 0, };
  gboolean retval = FALSE;

  if (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)) == G_TYPE_BOXED)
    {
      JsonNodeType node_type = json_node_get_node_type (node);
      GType boxed_type = G_VALUE_TYPE (value);

      if (json_boxed_can_deserialize (boxed_type, node_type))
        {
          gpointer boxed = json_boxed_deserialize (boxed_type, node);

          g_value_take_boxed (value, boxed);

          return TRUE;
        }
    }

  switch (JSON_NODE_TYPE (node))
    {
    case JSON_NODE_OBJECT:
      if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_OBJECT))
        {
          GObject *object;

          object = json_gobject_new (G_VALUE_TYPE (value), json_node_get_object (node));
          if (object != NULL)
            g_value_take_object (value, object);
          else
            g_value_set_object (value, NULL);

          retval = TRUE;
        }
      break;

    case JSON_NODE_ARRAY:
      if (G_VALUE_HOLDS (value, G_TYPE_STRV))
        {
          JsonArray *array = json_node_get_array (node);
          guint i, array_len = json_array_get_length (array);
          GPtrArray *str_array = g_ptr_array_sized_new (array_len + 1);

          for (i = 0; i < array_len; i++)
            {
              JsonNode *val = json_array_get_element (array, i);

              if (JSON_NODE_TYPE (val) != JSON_NODE_VALUE)
                continue;

              if (json_node_get_string (val) != NULL)
                g_ptr_array_add (str_array, (gpointer) json_node_get_string (val));
            }

          g_ptr_array_add (str_array, NULL);

          g_value_set_boxed (value, str_array->pdata);

          g_ptr_array_free (str_array, TRUE);

          retval = TRUE;
        }
      break;

    case JSON_NODE_VALUE:
      json_node_get_value (node, &node_value);
#if 0
      {
        gchar *node_str = g_strdup_value_contents (&node_value);
        g_debug ("%s: value type '%s' := node value type '%s' -> '%s'",
                 G_STRLOC,
                 g_type_name (G_VALUE_TYPE (value)),
                 g_type_name (G_VALUE_TYPE (&node_value)),
                 node_str);
        g_free (node_str);
      }
#endif

      switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)))
        {
        case G_TYPE_BOOLEAN:
        case G_TYPE_INT64:
        case G_TYPE_STRING:
	  if (G_VALUE_HOLDS (&node_value, G_VALUE_TYPE (value)))
	    {
	      g_value_copy (&node_value, value);
	      retval = TRUE;
	    }
          break;

        case G_TYPE_INT:
	  if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
	    {
	      g_value_set_int (value, (gint) g_value_get_int64 (&node_value));
	      retval = TRUE;
	    }
          break;

        case G_TYPE_CHAR:
	  if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
	    {
#if GLIB_CHECK_VERSION (2, 31, 0)
	      g_value_set_schar (value, (gchar) g_value_get_int64 (&node_value));
#else
              g_value_set_char (value, (gchar) g_value_get_int64 (&node_value));
#endif
	      retval = TRUE;
	    }
          break;

        case G_TYPE_UINT:
	  if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
	    {
	      g_value_set_uint (value, (guint) g_value_get_int64 (&node_value));
	      retval = TRUE;
	    }
          break;

        case G_TYPE_UCHAR:
	  if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
	    {
	      g_value_set_uchar (value, (guchar) g_value_get_int64 (&node_value));
	      retval = TRUE;
	    }
          break;

        case G_TYPE_DOUBLE:

	  if (G_VALUE_HOLDS (&node_value, G_TYPE_DOUBLE))
	    {
	      g_value_set_double (value, g_value_get_double (&node_value));
	      retval = TRUE;
	    }
	  else if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
	    {
	      g_value_set_double (value, (gdouble) g_value_get_int64 (&node_value));
	      retval = TRUE;
	    }

          break;

        case G_TYPE_FLOAT:
	  if (G_VALUE_HOLDS (&node_value, G_TYPE_DOUBLE))
	    {
	      g_value_set_float (value, (gfloat) g_value_get_double (&node_value));
	      retval = TRUE;
	    }
	  else if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
	    {
	      g_value_set_float (value, (gfloat) g_value_get_int64 (&node_value));
	      retval = TRUE;
	    }

          break;

        case G_TYPE_ENUM:
          {
            gint enum_value = 0;

            if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
              {
                enum_value = g_value_get_int64 (&node_value);
                retval = TRUE;
              }
            else if (G_VALUE_HOLDS (&node_value, G_TYPE_STRING))
              {
                retval = enum_from_string (G_VALUE_TYPE (value),
                                           g_value_get_string (&node_value),
                                           &enum_value);
              }

            if (retval)
              g_value_set_enum (value, enum_value);
          }
          break;

        case G_TYPE_FLAGS:
          {
            gint flags_value = 0;

            if (G_VALUE_HOLDS (&node_value, G_TYPE_INT64))
              {
                flags_value = g_value_get_int64 (&node_value);
                retval = TRUE;
              }
            else if (G_VALUE_HOLDS (&node_value, G_TYPE_STRING))
              {
                retval = flags_from_string (G_VALUE_TYPE (value),
                                            g_value_get_string (&node_value),
                                            &flags_value);
              }

            if (retval)
              g_value_set_flags (value, flags_value);
          }
          break;

        default:
          retval = FALSE;
          break;
        }

      g_value_unset (&node_value);
      break;

    case JSON_NODE_NULL:
      if (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)) == G_TYPE_STRING)
	{
	  g_value_set_string (value, NULL);
	  retval = TRUE;
	}
      else if (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value)) == G_TYPE_OBJECT)
	{
	  g_value_set_object (value, NULL);
	  retval = TRUE;
	}
      else
	retval = FALSE;

      break;
    }

  return retval;
}

JsonNode *
json_serialize_pspec (const GValue *real_value,
                      GParamSpec   *pspec)
{
  JsonNode *retval = NULL;
  GValue value = { 0, };
  JsonNodeType node_type;

  switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (real_value)))
    {
    case G_TYPE_INT64:
    case G_TYPE_BOOLEAN:
    case G_TYPE_DOUBLE:
      /* JSON native types */
      retval = json_node_new (JSON_NODE_VALUE);
      g_value_init (&value, G_VALUE_TYPE (real_value));
      g_value_copy (real_value, &value);
      json_node_set_value (retval, &value);
      g_value_unset (&value);
      break;

    case G_TYPE_STRING:
      /* strings might be NULL, so we handle it differently */
      if (!g_value_get_string (real_value))
        retval = json_node_new (JSON_NODE_NULL);
      else
        {
          retval = json_node_new (JSON_NODE_VALUE);
          json_node_set_string (retval, g_value_get_string (real_value));
          break;
        }
      break;

    case G_TYPE_INT:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (retval, g_value_get_int (real_value));
      break;

    case G_TYPE_FLOAT:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_double (retval, g_value_get_float (real_value));
      break;

    case G_TYPE_BOXED:
      if (G_VALUE_HOLDS (real_value, G_TYPE_STRV))
        {
          gchar **strv = g_value_get_boxed (real_value);
          gint i, strv_len;
          JsonArray *array;

          strv_len = g_strv_length (strv);
          array = json_array_sized_new (strv_len);

          for (i = 0; i < strv_len; i++)
            {
              JsonNode *str = json_node_new (JSON_NODE_VALUE);

              json_node_set_string (str, strv[i]);
              json_array_add_element (array, str);
            }

          retval = json_node_new (JSON_NODE_ARRAY);
          json_node_take_array (retval, array);
        }
      else if (json_boxed_can_serialize (G_VALUE_TYPE (real_value), &node_type))
        {
          gpointer boxed = g_value_get_boxed (real_value);

          retval = json_boxed_serialize (G_VALUE_TYPE (real_value), boxed);
        }
      else
        g_warning ("Boxed type '%s' is not handled by JSON-GLib",
                   g_type_name (G_VALUE_TYPE (real_value)));
      break;

    case G_TYPE_UINT:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (retval, g_value_get_uint (real_value));
      break;

    case G_TYPE_LONG:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (retval, g_value_get_long (real_value));
      break;

    case G_TYPE_ULONG:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (retval, g_value_get_long (real_value));
      break;

    case G_TYPE_CHAR:
      retval = json_node_new (JSON_NODE_VALUE);
#if GLIB_CHECK_VERSION (2, 31, 0)
      json_node_set_int (retval, g_value_get_schar (real_value));
#else
      json_node_set_int (retval, g_value_get_char (real_value));
#endif
      break;

    case G_TYPE_UCHAR:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (retval, g_value_get_uchar (real_value));
      break;

    case G_TYPE_ENUM:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (retval, g_value_get_enum (real_value));
      break;

    case G_TYPE_FLAGS:
      retval = json_node_new (JSON_NODE_VALUE);
      json_node_set_int (retval, g_value_get_flags (real_value));
      break;

    case G_TYPE_OBJECT:
      {
        GObject *object = g_value_get_object (real_value);

        if (object != NULL)
          {
            retval = json_node_new (JSON_NODE_OBJECT);
            json_node_take_object (retval, json_gobject_dump (object));
          }
        else
          retval = json_node_new (JSON_NODE_NULL);
      }
      break;

    case G_TYPE_NONE:
      retval = json_node_new (JSON_NODE_NULL);
      break;

    default:
      g_warning ("Unsupported type `%s'", g_type_name (G_VALUE_TYPE (real_value)));
      break;
    }

  return retval;
}

/**
 * json_gobject_deserialize:
 * @gtype: the type of the #GObject to create
 * @node: a #JsonNode of type %JSON_NODE_OBJECT describing the
 *   instance of type @gtype
 *
 * Creates a new #GObject of type @gtype, and constructs it
 * using the members of the passed #JsonObject
 *
 * Return value: (transfer full): The newly created #GObject
 *   instance. Use g_object_unref() to free the resources
 *   allocated by this function
 *
 * Since: 0.10
 */
GObject *
json_gobject_deserialize (GType     gtype,
                          JsonNode *node)
{
  g_return_val_if_fail (g_type_is_a (gtype, G_TYPE_OBJECT), NULL);
  g_return_val_if_fail (JSON_NODE_TYPE (node) == JSON_NODE_OBJECT, NULL);

  return json_gobject_new (gtype, json_node_get_object (node));
}

/**
 * json_gobject_serialize:
 * @gobject: a #GObject
 *
 * Creates a #JsonNode representing the passed #GObject
 * instance. Each member of the returned JSON object will
 * map to a property of the #GObject
 *
 * Return value: (transfer full): the newly created #JsonNode
 *   of type %JSON_NODE_OBJECT. Use json_node_free() to free
 *   the resources allocated by this function
 *
 * Since: 0.10
 */
JsonNode *
json_gobject_serialize (GObject *gobject)
{
  JsonNode *retval;

  g_return_val_if_fail (G_IS_OBJECT (gobject), NULL);

  retval = json_node_new (JSON_NODE_OBJECT);
  json_node_take_object (retval, json_gobject_dump (gobject));

  return retval;
}

/**
 * json_construct_gobject:
 * @gtype: the #GType of object to construct
 * @data: a JSON data stream
 * @length: length of the data stream
 * @error: return location for a #GError, or %NULL
 *
 * Deserializes a JSON data stream and creates the corresponding
 * #GObject class. If @gtype implements the #JsonSerializableIface
 * interface, it will be asked to deserialize all the JSON members
 * into the respective properties; otherwise, the default implementation
 * will be used to translate the compatible JSON native types.
 *
 * Note: the JSON data stream must be an object declaration.
 *
 * Return value: (transfer full): a #GObject or %NULL
 *
 * Since: 0.4
 *
 * Deprecated: 0.10: Use json_gobject_from_data() instead
 */
GObject *
json_construct_gobject (GType         gtype,
                        const gchar  *data,
                        gsize         length,
                        GError      **error)
{
  return json_gobject_from_data (gtype, data, strlen (data), error);
}

/**
 * json_gobject_from_data:
 * @gtype: the #GType of object to construct
 * @data: a JSON data stream
 * @length: length of the data stream, or -1 if it is NUL-terminated
 * @error: return location for a #GError, or %NULL
 *
 * Deserializes a JSON data stream and creates the corresponding
 * #GObject class. If @gtype implements the #JsonSerializableIface
 * interface, it will be asked to deserialize all the JSON members
 * into the respective properties; otherwise, the default implementation
 * will be used to translate the compatible JSON native types.
 *
 * Note: the JSON data stream must be an object declaration.
 *
 * Return value: (transfer full): a #GObject or %NULL
 *
 * Since: 0.10
 */
GObject *
json_gobject_from_data (GType         gtype,
                        const gchar  *data,
                        gssize        length,
                        GError      **error)
{
  JsonParser *parser;
  JsonNode *root;
  GError *parse_error;
  GObject *retval;

  g_return_val_if_fail (gtype != G_TYPE_INVALID, NULL);
  g_return_val_if_fail (data != NULL, NULL);

  if (length < 0)
    length = strlen (data);

  parser = json_parser_new ();

  parse_error = NULL;
  json_parser_load_from_data (parser, data, length, &parse_error);
  if (parse_error)
    {
      g_propagate_error (error, parse_error);
      g_object_unref (parser);
      return NULL;
    }

  root = json_parser_get_root (parser);
  if (root == NULL || JSON_NODE_TYPE (root) != JSON_NODE_OBJECT)
    {
      /* translators: the %s is the name of the data structure */
      g_set_error (error, JSON_PARSER_ERROR,
                   JSON_PARSER_ERROR_PARSE,
                   _("Expecting a JSON object, but the root node is of type `%s'"),
                   json_node_type_name (root));
      g_object_unref (parser);
      return NULL;
    }

  retval = json_gobject_deserialize (gtype, root);

  g_object_unref (parser);

  return retval;
}

/**
 * json_serialize_gobject:
 * @gobject: a #GObject
 * @length: (out): return value for the length of the buffer, or %NULL
 *
 * Serializes a #GObject into a JSON data stream. If @gobject implements
 * the #JsonSerializableIface interface, it will be asked to serizalize all
 * its properties; otherwise, the default implementation will be use to
 * translate the compatible types into JSON native types.
 *
 * Return value: a JSON data stream representing the passed #GObject
 *
 * Deprecated: 0.10: Use json_gobject_to_data() instead
 */
gchar *
json_serialize_gobject (GObject *gobject,
                        gsize   *length)
{
  return json_gobject_to_data (gobject, length);
}

/**
 * json_gobject_to_data:
 * @gobject: a #GObject
 * @length: (out): return value for the length of the buffer, or %NULL
 *
 * Serializes a #GObject into a JSON data stream, iterating recursively
 * over each property.
 *
 * If @gobject implements the #JsonSerializableIface interface, it will
 * be asked to serialize all its properties; otherwise, the default
 * implementation will be use to translate the compatible types into
 * JSON native types.
 *
 * Return value: a JSON data stream representing the passed #GObject
 *
 * Since: 0.10
 */
gchar *
json_gobject_to_data (GObject *gobject,
                      gsize   *length)
{
  JsonGenerator *gen;
  JsonNode *root;
  gchar *data;

  g_return_val_if_fail (G_OBJECT (gobject), NULL);

  root = json_gobject_serialize (gobject);

  gen = g_object_new (JSON_TYPE_GENERATOR,
                      "root", root,
                      "pretty", TRUE,
                      "indent", 2,
                      NULL);

  data = json_generator_to_data (gen, length);
  g_object_unref (gen);

  json_node_free (root);

  return data;
}
