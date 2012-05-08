/* json-reader.h - JSON cursor parser
 * 
 * This file is part of JSON-GLib
 * Copyright (C) 2010  Intel Corp.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   Emmanuele Bassi  <ebassi@linux.intel.com>
 */

/**
 * SECTION:json-reader
 * @Title: JsonReader
 * @short_description: A cursor-based parser
 *
 * #JsonReader provides a simple, cursor-based API for parsing a JSON DOM. It
 * is similar, in spirit, to the XML Reader API.
 *
 * In case of error, #JsonReader will be set in an error state; all subsequent
 * calls will simply be ignored until a function that resets the error state is
 * called, e.g.:
 *
 * |[
 * /&ast; ask for the 7th element; if the element does not exist, the
 *  &ast; reader will be put in an error state
 *  &ast;/
 * json_reader_read_element (reader, 6);
 *
 * /&ast; in case of error, this will return NULL, otherwise it will
 *  &ast; return the value of the element
 *  &ast;/
 * str = json_reader_get_string_value (value);
 *
 * /&ast; this function resets the error state if any was set &ast;/
 * json_reader_end_element (reader);
 * ]|
 *
 * If you want to detect the error state as soon as possible, you can use
 * json_reader_get_error():
 *
 * |[
 * /&ast; like the example above, but in this case we print out the
 *  &ast; error immediately
 *  &ast;/
 * if (!json_reader_read_element (reader, 6))
 *   {
 *     const GError *error = json_reader_get_error (reader);
 *     g_print ("Unable to read the element: %s", error->message);
 *   }
 * ]|
 *
 * #JsonReader is available since JSON-GLib 0.12.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include <glib/gi18n-lib.h>

#include "json-reader.h"

#include "json-types-private.h"

#include "json-debug.h"

#define json_reader_return_if_error_set(r)      G_STMT_START {  \
        if (((JsonReader *) (r))->priv->error != NULL)          \
          return;                               } G_STMT_END

#define json_reader_return_val_if_error_set(r,v) G_STMT_START {  \
        if (((JsonReader *) (r))->priv->error != NULL)           \
          return (v);                           } G_STMT_END

struct _JsonReaderPrivate
{
  JsonNode *root;

  JsonNode *current_node;
  JsonNode *previous_node;

  gchar *current_member;

  GError *error;
};

enum
{
  PROP_0,

  PROP_ROOT,

  PROP_LAST
};

static GParamSpec *reader_properties[PROP_LAST] = { NULL, };

G_DEFINE_TYPE (JsonReader, json_reader, G_TYPE_OBJECT);

static void
json_reader_finalize (GObject *gobject)
{
  JsonReaderPrivate *priv = JSON_READER (gobject)->priv;

  if (priv->root != NULL)
    json_node_free (priv->root);

  if (priv->error != NULL)
    g_clear_error (&priv->error);

  g_free (priv->current_member);

  G_OBJECT_CLASS (json_reader_parent_class)->finalize (gobject);
}

static void
json_reader_set_property (GObject      *gobject,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  switch (prop_id)
    {
    case PROP_ROOT:
      json_reader_set_root (JSON_READER (gobject), g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
json_reader_get_property (GObject    *gobject,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  switch (prop_id)
    {
    case PROP_ROOT:
      g_value_set_boxed (value, JSON_READER (gobject)->priv->root);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
json_reader_class_init (JsonReaderClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (JsonReaderPrivate));

  /**
   * JsonReader:root:
   *
   * The root of the JSON tree that the #JsonReader should read.
   *
   * Since: 0.12
   */
  reader_properties[PROP_ROOT] =
    g_param_spec_boxed ("root",
                        "Root Node",
                        "The root of the tree to read",
                        JSON_TYPE_NODE,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT |
                        G_PARAM_STATIC_STRINGS);

  gobject_class->finalize = json_reader_finalize;
  gobject_class->set_property = json_reader_set_property;
  gobject_class->get_property = json_reader_get_property;
  g_object_class_install_properties (gobject_class, PROP_LAST, reader_properties);
}

static void
json_reader_init (JsonReader *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, JSON_TYPE_READER,
                                            JsonReaderPrivate);
}

GQuark
json_reader_error_quark (void)
{
  return g_quark_from_static_string ("json-reader-error");
}

/**
 * json_reader_new:
 * @node: (allow-none): a #JsonNode, or %NULL
 *
 * Creates a new #JsonReader. You can use this object to read the contents of
 * the JSON tree starting from @node
 *
 * Return value: the newly created #JsonReader. Use g_object_unref() to
 *   release the allocated resources when done
 *
 * Since: 0.12
 */
JsonReader *
json_reader_new (JsonNode *node)
{
  return g_object_new (JSON_TYPE_READER, "root", node, NULL);
}

/*
 * json_reader_unset_error:
 * @reader: a #JsonReader
 *
 * Unsets the error state of @reader, if set
 */
static inline void
json_reader_unset_error (JsonReader *reader)
{
  if (reader->priv->error != NULL)
    g_clear_error (&(reader->priv->error));
}

/**
 * json_reader_set_root:
 * @reader: a #JsonReader
 * @root: (allow-none): a #JsonNode
 *
 * Sets the root #JsonNode to be read by @reader. The @reader will take
 * a copy of @root
 *
 * If another #JsonNode is currently set as root, it will be replaced.
 *
 * Since: 0.12
 */
void
json_reader_set_root (JsonReader *reader,
                      JsonNode   *root)
{
  JsonReaderPrivate *priv;

  g_return_if_fail (JSON_IS_READER (reader));

  priv = reader->priv;

  if (priv->root == root)
    return;

  if (priv->root != NULL)
    {
      json_node_free (priv->root);
      priv->root = NULL;
      priv->current_node = NULL;
      priv->previous_node = NULL;
    }

  if (root != NULL)
    {
      priv->root = json_node_copy (root);
      priv->current_node = priv->root;
      priv->previous_node = NULL;
    }

  g_object_notify_by_pspec (G_OBJECT (reader), reader_properties[PROP_ROOT]);
}

/*
 * json_reader_ser_error:
 * @reader: a #JsonReader
 * @error_code: the #JsonReaderError code for the error
 * @error_fmt: format string
 * @Varargs: list of arguments for the @error_fmt string
 *
 * Sets the error state of @reader using the given error code
 * and string
 *
 * Return value: %FALSE, to be used to return immediately from
 *   the caller function
 */
static gboolean
json_reader_set_error (JsonReader      *reader,
                       JsonReaderError  error_code,
                       const gchar     *error_fmt,
                       ...)
{
  JsonReaderPrivate *priv = reader->priv;
  va_list args;
  gchar *error_msg;

  if (priv->error != NULL)
    g_clear_error (&priv->error);

  va_start (args, error_fmt);
  error_msg = g_strdup_vprintf (error_fmt, args);
  va_end (args);

  g_set_error_literal (&priv->error, JSON_READER_ERROR,
                       error_code,
                       error_msg);

  g_free (error_msg);

  return FALSE;
}

/**
 * json_reader_get_error:
 * @reader: a #JsonReader
 *
 * Retrieves the #GError currently set on @reader, if the #JsonReader
 * is in error state
 *
 * Return value: (transfer none): the pointer to the error, or %NULL
 *
 * Since: 0.12
 */
const GError *
json_reader_get_error (JsonReader *reader)
{
  g_return_val_if_fail (JSON_IS_READER (reader), FALSE);

  return reader->priv->error;
}

/**
 * json_reader_is_array:
 * @reader: a #JsonReader
 *
 * Checks whether the @reader is currently on an array
 *
 * Return value: %TRUE if the #JsonReader is on an array, and %FALSE
 *   otherwise
 *
 * Since: 0.12
 */
gboolean
json_reader_is_array (JsonReader *reader)
{
  g_return_val_if_fail (JSON_IS_READER (reader), FALSE);
  json_reader_return_val_if_error_set (reader, FALSE);

  if (reader->priv->current_node == NULL)
    return FALSE;

  return JSON_NODE_HOLDS_ARRAY (reader->priv->current_node);
}

/**
 * json_reader_is_object:
 * @reader: a #JsonReader
 *
 * Checks whether the @reader is currently on an object
 *
 * Return value: %TRUE if the #JsonReader is on an object, and %FALSE
 *   otherwise
 *
 * Since: 0.12
 */
gboolean
json_reader_is_object (JsonReader *reader)
{
  g_return_val_if_fail (JSON_IS_READER (reader), FALSE);
  json_reader_return_val_if_error_set (reader, FALSE);

  if (reader->priv->current_node == NULL)
    return FALSE;

  return JSON_NODE_HOLDS_OBJECT (reader->priv->current_node);
}

/**
 * json_reader_is_value:
 * @reader: a #JsonReader
 *
 * Checks whether the @reader is currently on a value
 *
 * Return value: %TRUE if the #JsonReader is on a value, and %FALSE
 *   otherwise
 *
 * Since: 0.12
 */
gboolean
json_reader_is_value (JsonReader *reader)
{
  g_return_val_if_fail (JSON_IS_READER (reader), FALSE);
  json_reader_return_val_if_error_set (reader, FALSE);

  if (reader->priv->current_node == NULL)
    return FALSE;

  return JSON_NODE_HOLDS_VALUE (reader->priv->current_node);
}

/**
 * json_reader_read_element:
 * @reader: a #JsonReader
 * @index_: the index of the element
 *
 * Advances the cursor of @reader to the element @index_ of the array
 * or the object at the current position.
 *
 * You can use the json_reader_get_value* family of functions to retrieve
 * the value of the element; for instance:
 *
 * |[
 * json_reader_read_element (reader, 0);
 * int_value = json_reader_get_int_value (reader);
 * ]|
 *
 * After reading the value, json_reader_end_element() should be called to
 * reposition the cursor inside the #JsonReader, e.g.:
 *
 * |[
 * json_reader_read_element (reader, 1);
 * str_value = json_reader_get_string_value (reader);
 * json_reader_end_element (reader);
 *
 * json_reader_read_element (reader, 2);
 * str_value = json_reader_get_string_value (reader);
 * json_reader_end_element (reader);
 * ]|
 *
 * If @reader is not currently on an array or an object, or if the @index_ is
 * bigger than the size of the array or the object, the #JsonReader will be
 * put in an error state until json_reader_end_element() is called.
 *
 * Return value: %TRUE on success, and %FALSE otherwise
 *
 * Since: 0.12
 */
gboolean
json_reader_read_element (JsonReader *reader,
                          guint       index_)
{
  JsonReaderPrivate *priv;

  g_return_val_if_fail (JSON_READER (reader), FALSE);
  json_reader_return_val_if_error_set (reader, FALSE);

  priv = reader->priv;

  if (priv->current_node == NULL)
    priv->current_node = priv->root;

  if (!(JSON_NODE_HOLDS_ARRAY (priv->current_node) ||
        JSON_NODE_HOLDS_OBJECT (priv->current_node)))
    return json_reader_set_error (reader, JSON_READER_ERROR_NO_ARRAY,
                                  _("The current node is of type '%s', but "
                                    "an array or an object was expected."),
                                  json_node_type_name (priv->current_node));

  switch (json_node_get_node_type (priv->current_node))
    {
    case JSON_NODE_ARRAY:
      {
        JsonArray *array = json_node_get_array (priv->current_node);

        if (index_ >= json_array_get_length (array))
          return json_reader_set_error (reader, JSON_READER_ERROR_INVALID_INDEX,
                                        _("The index '%d' is greater than the size "
                                          "of the array at the current position."),
                                        index_);

        priv->previous_node = priv->current_node;
        priv->current_node = json_array_get_element (array, index_);
      }
      break;

    case JSON_NODE_OBJECT:
      {
        JsonObject *object = json_node_get_object (priv->current_node);
        GList *members;
        const gchar *name;

        if (index_ >= json_object_get_size (object))
          return json_reader_set_error (reader, JSON_READER_ERROR_INVALID_INDEX,
                                        _("The index '%d' is greater than the size "
                                          "of the object at the current position."),
                                        index_);

        priv->previous_node = priv->current_node;
        g_free (priv->current_member);

        members = json_object_get_members (object);
        name = g_list_nth_data (members, index_);

        priv->current_node = json_object_get_member (object, name);
        priv->current_member = g_strdup (name);

        g_list_free (members);
      }
      break;

    default:
      g_assert_not_reached ();
      return FALSE;
    }

  return TRUE;
}

/**
 * json_reader_end_element:
 * @reader: a #JsonReader
 *
 * Moves the cursor back to the previous node after being positioned
 * inside an array
 *
 * This function resets the error state of @reader, if any was set
 *
 * Since: 0.12
 */
void
json_reader_end_element (JsonReader *reader)
{
  JsonReaderPrivate *priv;
  JsonNode *tmp;

  g_return_if_fail (JSON_IS_READER (reader));

  json_reader_unset_error (reader);

  priv = reader->priv;

  if (priv->previous_node != NULL)
    tmp = json_node_get_parent (priv->previous_node);
  else
    tmp = NULL;

  g_free (priv->current_member);
  priv->current_member = NULL;

  priv->current_node = priv->previous_node;
  priv->previous_node = tmp;
}

/**
 * json_reader_count_elements:
 * @reader: a #JsonReader
 *
 * Counts the elements of the current position, if @reader is
 * positioned on an array
 *
 * Return value: the number of elements, or -1. In case of failure
 *   the #JsonReader is set in an error state
 *
 * Since: 0.12
 */
gint
json_reader_count_elements (JsonReader *reader)
{
  JsonReaderPrivate *priv;

  g_return_val_if_fail (JSON_IS_READER (reader), -1);

  priv = reader->priv;

  if (priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return -1;
    }

  if (!JSON_NODE_HOLDS_ARRAY (priv->current_node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_ARRAY,
                             _("The current position holds a '%s' and not an array"),
                             json_node_type_get_name (JSON_NODE_TYPE (priv->current_node)));
      return -1;
    }

  return json_array_get_length (json_node_get_array (priv->current_node));
}

/**
 * json_reader_read_member:
 * @reader: a #JsonReader
 * @member_name: the name of the member to read
 *
 * Advances the cursor of @reader to the @member_name of the object at the
 * current position.
 *
 * You can use the json_reader_get_value* family of functions to retrieve
 * the value of the member; for instance:
 *
 * |[
 * json_reader_read_member (reader, "width");
 * width = json_reader_get_int_value (reader);
 * ]|
 *
 * After reading the value, json_reader_end_member() should be called to
 * reposition the cursor inside the #JsonReader, e.g.:
 *
 * |[
 * json_reader_read_member (reader, "author");
 * author = json_reader_get_string_value (reader);
 * json_reader_end_element (reader);
 *
 * json_reader_read_element (reader, "title");
 * title = json_reader_get_string_value (reader);
 * json_reader_end_element (reader);
 * ]|
 *
 * If @reader is not currently on an object, or if the @member_name is not
 * defined in the object, the #JsonReader will be put in an error state until
 * json_reader_end_member() is called.
 *
 * Return value: %TRUE on success, and %FALSE otherwise
 *
 * Since: 0.12
 */
gboolean
json_reader_read_member (JsonReader  *reader,
                         const gchar *member_name)
{
  JsonReaderPrivate *priv;
  JsonObject *object;

  g_return_val_if_fail (JSON_READER (reader), FALSE);
  g_return_val_if_fail (member_name != NULL, FALSE);
  json_reader_return_val_if_error_set (reader, FALSE);

  priv = reader->priv;

  if (priv->current_node == NULL)
    priv->current_node = priv->root;

  if (!JSON_NODE_HOLDS_OBJECT (priv->current_node))
    return json_reader_set_error (reader, JSON_READER_ERROR_NO_OBJECT,
                                  _("The current node is of type '%s', but "
                                    "an object was expected."),
                                  json_node_type_name (priv->current_node));

  object = json_node_get_object (priv->current_node);
  if (!json_object_has_member (object, member_name))
    return json_reader_set_error (reader, JSON_READER_ERROR_INVALID_MEMBER,
                                  _("The member '%s' is not defined in the "
                                    "object at the current position."),
                                  member_name);

  g_free (priv->current_member);

  priv->previous_node = priv->current_node;
  priv->current_node = json_object_get_member (object, member_name);
  priv->current_member = g_strdup (member_name);

  return TRUE;
}

/**
 * json_reader_end_member:
 * @reader: a #JsonReader
 *
 * Moves the cursor back to the previous node after being positioned
 * inside an object
 *
 * This function resets the error state of @reader, if any was set
 *
 * Since: 0.12
 */
void
json_reader_end_member (JsonReader *reader)
{
  JsonReaderPrivate *priv;
  JsonNode *tmp;

  g_return_if_fail (JSON_IS_READER (reader));

  json_reader_unset_error (reader);

  priv = reader->priv;

  if (priv->previous_node != NULL)
    tmp = json_node_get_parent (priv->previous_node);
  else
    tmp = NULL;

  g_free (priv->current_member);
  priv->current_member = NULL;

  priv->current_node = priv->previous_node;
  priv->previous_node = tmp;
}

/**
 * json_reader_list_members:
 * @reader: a #JsonReader
 *
 * Retrieves a list of member names from the current position, if @reader
 * is positioned on an object.
 *
 * Return value: (transfer full): a newly allocated, %NULL-terminated
 *   array of strings holding the members name. Use g_strfreev() when
 *   done.
 *
 * Since: 0.14
 */
gchar **
json_reader_list_members (JsonReader *reader)
{
  JsonReaderPrivate *priv;
  GList *members, *l;
  gchar **retval;
  gint i;

  g_return_val_if_fail (JSON_IS_READER (reader), NULL);

  priv = reader->priv;

  if (priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return NULL;
    }

  if (!JSON_NODE_HOLDS_OBJECT (priv->current_node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_OBJECT,
                             _("The current position holds a '%s' and not an object"),
                             json_node_type_get_name (JSON_NODE_TYPE (priv->current_node)));
      return NULL;
    }

  members = json_object_get_members (json_node_get_object (priv->current_node));
  if (members == NULL)
    return NULL;

  retval = g_new (gchar*, g_list_length (members) + 1);
  for (l = members, i = 0; l != NULL; l = l->next, i += 1)
    retval[i] = g_strdup (l->data);

  retval[i] = NULL;

  g_list_free (members);

  return retval;
}

/**
 * json_reader_count_members:
 * @reader: a #JsonReader
 *
 * Counts the members of the current position, if @reader is
 * positioned on an object
 *
 * Return value: the number of members, or -1. In case of failure
 *   the #JsonReader is set in an error state
 *
 * Since: 0.12
 */
gint
json_reader_count_members (JsonReader *reader)
{
  JsonReaderPrivate *priv;

  g_return_val_if_fail (JSON_IS_READER (reader), -1);

  priv = reader->priv;

  if (priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return -1;
    }

  if (!JSON_NODE_HOLDS_OBJECT (priv->current_node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_OBJECT,
                             _("The current position holds a '%s' and not an object"),
                             json_node_type_get_name (JSON_NODE_TYPE (priv->current_node)));
      return -1;
    }

  return json_object_get_size (json_node_get_object (priv->current_node));
}

/**
 * json_reader_get_value:
 * @reader: a #JsonReader
 *
 * Retrieves the #JsonNode of the current position of @reader
 *
 * Return value: (transfer none): a #JsonNode, or %NULL. The returned node
 *   is owned by the #JsonReader and it should not be modified or freed
 *   directly
 *
 * Since: 0.12
 */
JsonNode *
json_reader_get_value (JsonReader *reader)
{
  JsonNode *node;

  g_return_val_if_fail (JSON_IS_READER (reader), NULL);
  json_reader_return_val_if_error_set (reader, NULL);

  if (reader->priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return NULL;
    }

  node = reader->priv->current_node;

  if (!JSON_NODE_HOLDS_VALUE (node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_VALUE,
                             _("The current position holds a '%s' and not a value"),
                             json_node_type_get_name (JSON_NODE_TYPE (node)));
      return NULL;
    }

  return reader->priv->current_node;
}

/**
 * json_reader_get_int_value:
 * @reader: a #JsonReader
 *
 * Retrieves the integer value of the current position of @reader
 *
 * Return value: the integer value
 *
 * Since: 0.12
 */
gint64
json_reader_get_int_value (JsonReader *reader)
{
  JsonNode *node;

  g_return_val_if_fail (JSON_IS_READER (reader), 0);
  json_reader_return_val_if_error_set (reader, 0);

  if (reader->priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return 0;
    }

  node = reader->priv->current_node;

  if (!JSON_NODE_HOLDS_VALUE (node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_VALUE,
                             _("The current position holds a '%s' and not a value"),
                             json_node_type_get_name (JSON_NODE_TYPE (node)));
      return 0;
    }

  if (json_node_get_value_type (node) != G_TYPE_INT64)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_TYPE,
                             _("The current position does not hold an integer type"));
      return 0;
    }

  return json_node_get_int (reader->priv->current_node);
}

/**
 * json_reader_get_double_value:
 * @reader: a #JsonReader
 *
 * Retrieves the floating point value of the current position of @reader
 *
 * Return value: the floating point value
 *
 * Since: 0.12
 */
gdouble
json_reader_get_double_value (JsonReader *reader)
{
  JsonNode *node;

  g_return_val_if_fail (JSON_IS_READER (reader), 0.0);
  json_reader_return_val_if_error_set (reader, 0.0);

  if (reader->priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return 0.0;
    }

  node = reader->priv->current_node;

  if (!JSON_NODE_HOLDS_VALUE (node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_VALUE,
                             _("The current position holds a '%s' and not a value"),
                             json_node_type_get_name (JSON_NODE_TYPE (node)));
      return 0.0;
    }

  if (json_node_get_value_type (node) != G_TYPE_DOUBLE)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_TYPE,
                             _("The current position does not hold a floating point type"));
      return 0.0;
    }

  return json_node_get_double (reader->priv->current_node);
}

/**
 * json_reader_get_string_value:
 * @reader: a #JsonReader
 *
 * Retrieves the string value of the current position of @reader
 *
 * Return value: the string value
 *
 * Since: 0.12
 */
const gchar *
json_reader_get_string_value (JsonReader *reader)
{
  JsonNode *node;

  g_return_val_if_fail (JSON_IS_READER (reader), NULL);
  json_reader_return_val_if_error_set (reader, NULL);

  if (reader->priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return NULL;
    }

  node = reader->priv->current_node;

  if (!JSON_NODE_HOLDS_VALUE (node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_VALUE,
                             _("The current position holds a '%s' and not a value"),
                             json_node_type_get_name (JSON_NODE_TYPE (node)));
      return NULL;
    }

  if (json_node_get_value_type (node) != G_TYPE_STRING)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_TYPE,
                             _("The current position does not hold a string type"));
      return NULL;
    }

  return json_node_get_string (reader->priv->current_node);
}

/**
 * json_reader_get_boolean_value:
 * @reader: a #JsonReader
 *
 * Retrieves the boolean value of the current position of @reader
 *
 * Return value: the boolean value
 *
 * Since: 0.12
 */
gboolean
json_reader_get_boolean_value (JsonReader *reader)
{
  JsonNode *node;

  g_return_val_if_fail (JSON_IS_READER (reader), FALSE);
  json_reader_return_val_if_error_set (reader, FALSE);

  if (reader->priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return FALSE;
    }

  node = reader->priv->current_node;

  if (!JSON_NODE_HOLDS_VALUE (node))
    {
      json_reader_set_error (reader, JSON_READER_ERROR_NO_VALUE,
                             _("The current position holds a '%s' and not a value"),
                             json_node_type_get_name (JSON_NODE_TYPE (node)));
      return FALSE;
    }

  if (json_node_get_value_type (node) != G_TYPE_BOOLEAN)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_TYPE,
                             _("The current position does not hold a boolean type"));
      return FALSE;
    }

  return json_node_get_boolean (node);
}

/**
 * json_reader_get_null_value:
 * @reader: a #JsonReader
 *
 * Checks whether the value of the current position of @reader is 'null'
 *
 * Return value: %TRUE if 'null' is set, and %FALSE otherwise
 *
 * Since: 0.12
 */
gboolean
json_reader_get_null_value (JsonReader *reader)
{
  g_return_val_if_fail (JSON_IS_READER (reader), FALSE);
  json_reader_return_val_if_error_set (reader, FALSE);

  if (reader->priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return FALSE;
    }

  return JSON_NODE_HOLDS_NULL (reader->priv->current_node);
}

/**
 * json_reader_get_member_name:
 * @reader: a #JsonReader
 *
 * Retrieves the name of the current member.
 *
 * Return value: (transfer none): the name of the member, or %NULL
 *
 * Since: 0.14
 */
const gchar *
json_reader_get_member_name (JsonReader *reader)
{
  g_return_val_if_fail (JSON_IS_READER (reader), NULL);
  json_reader_return_val_if_error_set (reader, NULL);

  if (reader->priv->current_node == NULL)
    {
      json_reader_set_error (reader, JSON_READER_ERROR_INVALID_NODE,
                             _("No node available at the current position"));
      return FALSE;
    }

  return reader->priv->current_member;
}
