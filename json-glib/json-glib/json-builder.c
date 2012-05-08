/* json-generator.c - JSON tree builder
 *
 * This file is part of JSON-GLib
 * Copyright (C) 2010  Luca Bruno <lethalman88@gmail.com>
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
 *   Luca Bruno  <lethalman88@gmail.com>
 */

/**
 * SECTION:json-builder
 * @Title: JsonBuilder
 * @short_description: Generates JSON trees
 * @See_Also: JsonGenerator
 *
 * #JsonBuilder provides an object for generating a JSON tree.
 * You can generate only one tree with one #JsonBuilder instance.
 *
 * The root of the JSON tree can be either a #JsonObject or a #JsonArray.
 * Thus the first call must necessarily be either
 * json_builder_begin_object() or json_builder_begin_array().
 *
 * For convenience to language bindings, #JsonBuilder returns itself from
 * most of functions, making it easy to chain function calls.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "json-types-private.h"

#include "json-builder.h"

#define JSON_BUILDER_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JSON_TYPE_BUILDER, JsonBuilderPrivate))

struct _JsonBuilderPrivate
{
  GQueue *stack;
  JsonNode *root;
};

typedef enum
{
  JSON_BUILDER_MODE_OBJECT,
  JSON_BUILDER_MODE_ARRAY,
  JSON_BUILDER_MODE_MEMBER
} JsonBuilderMode;

typedef struct
{
  JsonBuilderMode mode;

  union
  {
    JsonObject *object;
    JsonArray *array;
  } data;
  gchar *member_name;
} JsonBuilderState;

static void
json_builder_state_free (JsonBuilderState *state)
{
  if (G_LIKELY (state))
    {
      switch (state->mode)
        {
        case JSON_BUILDER_MODE_OBJECT:
        case JSON_BUILDER_MODE_MEMBER:
          json_object_unref (state->data.object);
          g_free (state->member_name);
          state->data.object = NULL;
          state->member_name = NULL;
          break;
        case JSON_BUILDER_MODE_ARRAY:
          json_array_unref (state->data.array);
          state->data.array = NULL;
          break;
        default:
          g_assert_not_reached ();
        }

      g_slice_free (JsonBuilderState, state);
    }
}

G_DEFINE_TYPE (JsonBuilder, json_builder, G_TYPE_OBJECT);

static void
json_builder_free_all_state (JsonBuilder *builder)
{
  JsonBuilderState *state;

  while (!g_queue_is_empty (builder->priv->stack))
    {
      state = g_queue_pop_head (builder->priv->stack);
      json_builder_state_free (state);
    }

  if (builder->priv->root)
    {
      json_node_free (builder->priv->root);
      builder->priv->root = NULL;
    }
}

static void
json_builder_finalize (GObject *gobject)
{
  JsonBuilderPrivate *priv = JSON_BUILDER_GET_PRIVATE (gobject);

  json_builder_free_all_state (JSON_BUILDER (gobject));

  g_queue_free (priv->stack);
  priv->stack = NULL;

  G_OBJECT_CLASS (json_builder_parent_class)->finalize (gobject);
}

static void
json_builder_class_init (JsonBuilderClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (JsonBuilderPrivate));

  gobject_class->finalize = json_builder_finalize;
}

static void
json_builder_init (JsonBuilder *builder)
{
  JsonBuilderPrivate *priv;

  builder->priv = priv = JSON_BUILDER_GET_PRIVATE (builder);

  priv->stack = g_queue_new ();
  priv->root = NULL;
}

static inline JsonBuilderMode
json_builder_current_mode (JsonBuilder *builder)
{
  JsonBuilderState *state = g_queue_peek_head (builder->priv->stack);
  return state->mode;
}

static inline gboolean
json_builder_is_valid_add_mode (JsonBuilder *builder)
{
  JsonBuilderMode mode = json_builder_current_mode (builder);
  return mode == JSON_BUILDER_MODE_MEMBER || mode == JSON_BUILDER_MODE_ARRAY;
}

/**
 * json_builder_new:
 *
 * Creates a new #JsonBuilder. You can use this object to generate a
 * JSON tree and obtain the root #JsonNode<!-- -->s.
 *
 * Return value: the newly created #JsonBuilder instance
 */
JsonBuilder *
json_builder_new (void)
{
  return g_object_new (JSON_TYPE_BUILDER, NULL);
}

/**
 * json_builder_get_root:
 * @builder: a #JsonBuilder
 *
 * Returns the root of the current constructed tree, if the build is complete
 * (ie: all opened objects, object members and arrays are being closed).
 *
 * Return value: (transfer full): the #JsonNode, or %NULL if the build is not complete.
 *   Free the returned value with json_node_free().
 */
JsonNode *
json_builder_get_root (JsonBuilder *builder)
{
  JsonNode *root = NULL;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);

  if (builder->priv->root)
    root = json_node_copy (builder->priv->root);

  return root;
}

/**
 * json_builder_reset:
 * @builder: a #JsonBuilder
 *
 * Resets the state of the @builder back to its initial state.
 */
void
json_builder_reset (JsonBuilder *builder)
{
  g_return_if_fail (JSON_IS_BUILDER (builder));

  json_builder_free_all_state (builder);
}

/**
 * json_builder_begin_object:
 * @builder: a #JsonBuilder
 *
 * Opens a subobject inside the given @builder. When done adding members to
 * the subobject, json_builder_end_object() must be called.
 *
 * Can be called for first or only if the call is associated to an object member
 * or an array element.
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_begin_object (JsonBuilder *builder)
{
  JsonObject *object;
  JsonBuilderState *state;
  JsonBuilderState *cur_state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (builder->priv->root == NULL, NULL);
  g_return_val_if_fail (g_queue_is_empty (builder->priv->stack) || json_builder_is_valid_add_mode (builder), NULL);

  object = json_object_new ();
  cur_state = g_queue_peek_head (builder->priv->stack);
  if (cur_state)
    {
      switch (cur_state->mode)
        {
        case JSON_BUILDER_MODE_ARRAY:
          json_array_add_object_element (cur_state->data.array, json_object_ref (object));
          break;

        case JSON_BUILDER_MODE_MEMBER:
          json_object_set_object_member (cur_state->data.object, cur_state->member_name, json_object_ref (object));
          g_free (cur_state->member_name);
          cur_state->member_name = NULL;
          cur_state->mode = JSON_BUILDER_MODE_OBJECT;
          break;

        default:
          g_assert_not_reached ();
        }
    }

  state = g_slice_new (JsonBuilderState);
  state->data.object = object;
  state->member_name = NULL;
  state->mode = JSON_BUILDER_MODE_OBJECT;
  g_queue_push_head (builder->priv->stack, state);

  return builder;
}

/**
 * json_builder_end_object:
 * @builder: a #JsonBuilder
 *
 * Closes the subobject inside the given @builder that was opened by the most
 * recent call to json_builder_begin_object().
 *
 * Cannot be called after json_builder_set_member_name().
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_end_object (JsonBuilder *builder)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_current_mode (builder) == JSON_BUILDER_MODE_OBJECT, NULL);

  state = g_queue_pop_head (builder->priv->stack);

  if (g_queue_is_empty (builder->priv->stack))
    {
      builder->priv->root = json_node_new (JSON_NODE_OBJECT);
      json_node_take_object (builder->priv->root, json_object_ref (state->data.object));
    }

  json_builder_state_free (state);

  return builder;
}

/**
 * json_builder_begin_array:
 * @builder: a #JsonBuilder
 *
 * Opens a subarray inside the given @builder. When done adding members to
 * the subarray, json_builder_end_array() must be called.
 *
 * Can be called for first or only if the call is associated to an object member
 * or an array element.
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_begin_array (JsonBuilder *builder)
{
  JsonArray *array;
  JsonBuilderState *state;
  JsonBuilderState *cur_state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (builder->priv->root == NULL, NULL);
  g_return_val_if_fail (g_queue_is_empty (builder->priv->stack) || json_builder_is_valid_add_mode (builder), NULL);

  array = json_array_new ();
  cur_state = g_queue_peek_head (builder->priv->stack);
  if (cur_state)
    {
      switch (cur_state->mode)
        {
        case JSON_BUILDER_MODE_ARRAY:
          json_array_add_array_element (cur_state->data.array, json_array_ref (array));
          break;

        case JSON_BUILDER_MODE_MEMBER:
          json_object_set_array_member (cur_state->data.object, cur_state->member_name, json_array_ref (array));
          g_free (cur_state->member_name);
          cur_state->member_name = NULL;
          cur_state->mode = JSON_BUILDER_MODE_OBJECT;
          break;

        default:
          g_assert_not_reached ();
        }
    }

  state = g_slice_new (JsonBuilderState);
  state->data.array = array;
  state->mode = JSON_BUILDER_MODE_ARRAY;
  g_queue_push_head (builder->priv->stack, state);

  return builder;
}

/**
 * json_builder_end_array:
 * @builder: a #JsonBuilder
 *
 * Closes the subarray inside the given @builder that was opened by the most
 * recent call to json_builder_begin_array().
 *
 * Cannot be called after json_builder_set_member_name().
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_end_array (JsonBuilder *builder)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_current_mode (builder) == JSON_BUILDER_MODE_ARRAY, NULL);

  state = g_queue_pop_head (builder->priv->stack);

  if (g_queue_is_empty (builder->priv->stack))
    {
      builder->priv->root = json_node_new (JSON_NODE_ARRAY);
      json_node_take_array (builder->priv->root, json_array_ref (state->data.array));
    }

  json_builder_state_free (state);

  return builder;
}

/**
 * json_builder_set_member_name:
 * @builder: a #JsonBuilder
 * @member_name: the name of the member
 *
 * Set the name of the next member in an object. The next call must add a value,
 * open an object or an array.
 *
 * Can be called only if the call is associated to an object.
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_set_member_name (JsonBuilder *builder, const gchar *member_name)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (member_name != NULL, NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_current_mode (builder) == JSON_BUILDER_MODE_OBJECT, NULL);

  state = g_queue_peek_head (builder->priv->stack);
  state->member_name = g_strdup (member_name);
  state->mode = JSON_BUILDER_MODE_MEMBER;

  return builder;
}

/**
 * json_builder_add_value:
 * @builder: a #JsonBuilder
 * @node: the value of the member or element
 *
 * If called after json_builder_set_member_name(), sets @node as member of the
 * most recent opened object, otherwise @node is added as element of the most
 * recent opened array.
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_add_value (JsonBuilder *builder, JsonNode *node)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (node != NULL, NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_is_valid_add_mode (builder), NULL);

  state = g_queue_peek_head (builder->priv->stack);
  switch (state->mode)
    {
    case JSON_BUILDER_MODE_MEMBER:
      json_object_set_member (state->data.object, state->member_name, node);
      g_free (state->member_name);
      state->member_name = NULL;
      state->mode = JSON_BUILDER_MODE_OBJECT;
      break;

    case JSON_BUILDER_MODE_ARRAY:
      json_array_add_element (state->data.array, node);
      break;

    default:
      g_assert_not_reached ();
    }

  return builder;
}

/**
 * json_builder_add_int_value:
 * @builder: a #JsonBuilder
 * @value: the value of the member or element
 *
 * If called after json_builder_set_member_name(), sets @value as member of the
 * most recent opened object, otherwise @value is added as element of the most
 * recent opened array.
 *
 * See also: json_builder_add_value()
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_add_int_value (JsonBuilder *builder, gint64 value)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_is_valid_add_mode (builder), NULL);

  state = g_queue_peek_head (builder->priv->stack);
  switch (state->mode)
    {
    case JSON_BUILDER_MODE_MEMBER:
      json_object_set_int_member (state->data.object, state->member_name, value);
      g_free (state->member_name);
      state->member_name = NULL;
      state->mode = JSON_BUILDER_MODE_OBJECT;
      break;

    case JSON_BUILDER_MODE_ARRAY:
      json_array_add_int_element (state->data.array, value);
      break;

    default:
      g_assert_not_reached ();
    }

  return builder;
}

/**
 * json_builder_add_double_value:
 * @builder: a #JsonBuilder
 * @value: the value of the member or element
 *
 * If called after json_builder_set_member_name(), sets @value as member of the
 * most recent opened object, otherwise @value is added as element of the most
 * recent opened array.
 *
 * See also: json_builder_add_value()
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_add_double_value (JsonBuilder *builder, gdouble value)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_is_valid_add_mode (builder), NULL);

  state = g_queue_peek_head (builder->priv->stack);

  switch (state->mode)
    {
    case JSON_BUILDER_MODE_MEMBER:
      json_object_set_double_member (state->data.object, state->member_name, value);
      g_free (state->member_name);
      state->member_name = NULL;
      state->mode = JSON_BUILDER_MODE_OBJECT;
      break;

    case JSON_BUILDER_MODE_ARRAY:
      json_array_add_double_element (state->data.array, value);
      break;

    default:
      g_assert_not_reached ();
    }

  return builder;
}

/**
 * json_builder_add_boolean_value:
 * @builder: a #JsonBuilder
 * @value: the value of the member or element
 *
 * If called after json_builder_set_member_name(), sets @value as member of the
 * most recent opened object, otherwise @value is added as element of the most
 * recent opened array.
 *
 * See also: json_builder_add_value()
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_add_boolean_value (JsonBuilder *builder, gboolean value)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_is_valid_add_mode (builder), NULL);

  state = g_queue_peek_head (builder->priv->stack);

  switch (state->mode)
    {
    case JSON_BUILDER_MODE_MEMBER:
      json_object_set_boolean_member (state->data.object, state->member_name, value);
      g_free (state->member_name);
      state->member_name = NULL;
      state->mode = JSON_BUILDER_MODE_OBJECT;
      break;

    case JSON_BUILDER_MODE_ARRAY:
      json_array_add_boolean_element (state->data.array, value);
      break;

    default:
      g_assert_not_reached ();
    }

  return builder;
}

/**
 * json_builder_add_string_value:
 * @builder: a #JsonBuilder
 * @value: the value of the member or element
 *
 * If called after json_builder_set_member_name(), sets @value as member of the
 * most recent opened object, otherwise @value is added as element of the most
 * recent opened array.
 *
 * See also: json_builder_add_value()
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_add_string_value (JsonBuilder *builder, const gchar *value)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_is_valid_add_mode (builder), NULL);

  state = g_queue_peek_head (builder->priv->stack);

  switch (state->mode)
    {
    case JSON_BUILDER_MODE_MEMBER:
      json_object_set_string_member (state->data.object, state->member_name, value);
      g_free (state->member_name);
      state->member_name = NULL;
      state->mode = JSON_BUILDER_MODE_OBJECT;
      break;

    case JSON_BUILDER_MODE_ARRAY:
      json_array_add_string_element (state->data.array, value);
      break;

    default:
      g_assert_not_reached ();
    }

  return builder;
}

/**
 * json_builder_add_null_value:
 * @builder: a #JsonBuilder
 *
 * If called after json_builder_set_member_name(), sets null as member of the
 * most recent opened object, otherwise null is added as element of the most
 * recent opened array.
 *
 * See also: json_builder_add_value()
 *
 * Return value: (transfer none): the #JsonBuilder, or %NULL if the call was inconsistent
 */
JsonBuilder *
json_builder_add_null_value (JsonBuilder *builder)
{
  JsonBuilderState *state;

  g_return_val_if_fail (JSON_IS_BUILDER (builder), NULL);
  g_return_val_if_fail (!g_queue_is_empty (builder->priv->stack), NULL);
  g_return_val_if_fail (json_builder_is_valid_add_mode (builder), NULL);

  state = g_queue_peek_head (builder->priv->stack);

  switch (state->mode)
    {
    case JSON_BUILDER_MODE_MEMBER:
      json_object_set_null_member (state->data.object, state->member_name);
      g_free (state->member_name);
      state->member_name = NULL;
      state->mode = JSON_BUILDER_MODE_OBJECT;
      break;

    case JSON_BUILDER_MODE_ARRAY:
      json_array_add_null_element (state->data.array);
      break;

    default:
      g_assert_not_reached ();
    }

  return builder;
}
