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
 * SECTION:json-serializable
 * @short_description: Interface for serialize and deserialize special GObjects
 *
 * #JsonSerializable is an interface for #GObject classes that
 * allows controlling how the class is going to be serialized
 * or deserialized by json_construct_gobject() and
 * json_serialize_gobject() respectively.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include "json-types-private.h"
#include "json-gobject-private.h"
#include "json-debug.h"

/**
 * json_serializable_serialize_property:
 * @serializable: a #JsonSerializable object
 * @property_name: the name of the property
 * @value: the value of the property
 * @pspec: a #GParamSpec
 *
 * Asks a #JsonSerializable implementation to serialize a #GObject
 * property into a #JsonNode object.
 *
 * Return value: a #JsonNode containing the serialized property
 */
JsonNode *
json_serializable_serialize_property (JsonSerializable *serializable,
                                      const gchar      *property_name,
                                      const GValue     *value,
                                      GParamSpec       *pspec)
{
  JsonSerializableIface *iface;

  g_return_val_if_fail (JSON_IS_SERIALIZABLE (serializable), NULL);
  g_return_val_if_fail (property_name != NULL, NULL);
  g_return_val_if_fail (value != NULL, NULL);
  g_return_val_if_fail (pspec != NULL, NULL);

  iface = JSON_SERIALIZABLE_GET_IFACE (serializable);

  return iface->serialize_property (serializable, property_name, value, pspec);
}

/**
 * json_serializable_deserialize_property:
 * @serializable: a #JsonSerializable
 * @property_name: the name of the property
 * @value: (out): a pointer to an uninitialized #GValue
 * @pspec: a #GParamSpec
 * @property_node: a #JsonNode containing the serialized property
 *
 * Asks a #JsonSerializable implementation to deserialize the
 * property contained inside @property_node into @value.
 *
 * Return value: %TRUE if the property was successfully deserialized.
 */
gboolean
json_serializable_deserialize_property (JsonSerializable *serializable,
                                        const gchar      *property_name,
                                        GValue           *value,
                                        GParamSpec       *pspec,
                                        JsonNode         *property_node)
{
  JsonSerializableIface *iface;

  g_return_val_if_fail (JSON_IS_SERIALIZABLE (serializable), FALSE);
  g_return_val_if_fail (property_name != NULL, FALSE);
  g_return_val_if_fail (value != NULL, FALSE);
  g_return_val_if_fail (pspec != NULL, FALSE);
  g_return_val_if_fail (property_node != NULL, FALSE);

  iface = JSON_SERIALIZABLE_GET_IFACE (serializable);

  return iface->deserialize_property (serializable,
                                      property_name,
                                      value,
                                      pspec,
                                      property_node);
}

static gboolean
json_serializable_real_deserialize (JsonSerializable *serializable,
                                    const gchar      *name,
                                    GValue           *value,
                                    GParamSpec       *pspec,
                                    JsonNode         *node)
{
  JSON_NOTE (GOBJECT, "Default deserialization for property '%s'", pspec->name);
  return json_deserialize_pspec (value, pspec, node);
}

static JsonNode *
json_serializable_real_serialize (JsonSerializable *serializable,
                                  const gchar      *name,
                                  const GValue     *value,
                                  GParamSpec       *pspec)
{
  JSON_NOTE (GOBJECT, "Default serialization for property '%s'", pspec->name);

  if (g_param_value_defaults (pspec, (GValue *)value))
    return NULL;

  return json_serialize_pspec (value, pspec);
}

static GParamSpec *
json_serializable_real_find_property (JsonSerializable *serializable,
                                      const char       *name)
{
  return g_object_class_find_property (G_OBJECT_GET_CLASS (serializable), name);
}

static GParamSpec **
json_serializable_real_list_properties (JsonSerializable *serializable,
                                        guint            *n_pspecs)
{
  return g_object_class_list_properties (G_OBJECT_GET_CLASS (serializable), n_pspecs);
}

static void
json_serializable_real_set_property (JsonSerializable *serializable,
                                     GParamSpec       *pspec,
                                     const GValue     *value)
{
  g_object_set_property (G_OBJECT (serializable), pspec->name, value);
}

static void
json_serializable_real_get_property (JsonSerializable *serializable,
                                     GParamSpec       *pspec,
                                     GValue           *value)
{
  g_object_get_property (G_OBJECT (serializable), pspec->name, value);
}

/* typedef to satisfy G_DEFINE_INTERFACE's naming */
typedef JsonSerializableIface   JsonSerializableInterface;

static void
json_serializable_default_init (JsonSerializableInterface *iface)
{
  iface->serialize_property = json_serializable_real_serialize;
  iface->deserialize_property = json_serializable_real_deserialize;
  iface->find_property = json_serializable_real_find_property;
  iface->list_properties = json_serializable_real_list_properties;
  iface->set_property = json_serializable_real_set_property;
  iface->get_property = json_serializable_real_get_property;
}

G_DEFINE_INTERFACE (JsonSerializable, json_serializable, G_TYPE_OBJECT);

/**
 * json_serializable_default_serialize_property:
 * @serializable: a #JsonSerializable object
 * @property_name: the name of the property
 * @value: the value of the property
 * @pspec: a #GParamSpec
 *
 * Calls the default implementation of the #JsonSerializable
 * serialize_property() virtual function
 *
 * This function can be used inside a custom implementation
 * of the serialize_property() virtual function in lieu of:
 *
 * |[
 *   JsonSerializable *iface;
 *   JsonNode *node;
 *
 *   iface = g_type_default_interface_peek (JSON_TYPE_SERIALIZABLE);
 *   node = iface->serialize_property (serializable, property_name,
 *                                     value,
 *                                     pspec);
 * ]|
 *
 * Return value: (transfer full): a #JsonNode containing the serialized
 *   property
 *
 * Since: 0.10
 */
JsonNode *
json_serializable_default_serialize_property (JsonSerializable *serializable,
                                              const gchar      *property_name,
                                              const GValue     *value,
                                              GParamSpec       *pspec)
{
  g_return_val_if_fail (JSON_IS_SERIALIZABLE (serializable), NULL);
  g_return_val_if_fail (property_name != NULL, NULL);
  g_return_val_if_fail (value != NULL, NULL);
  g_return_val_if_fail (pspec != NULL, NULL);

  return json_serializable_real_serialize (serializable,
                                           property_name,
                                           value, pspec);
}

/**
 * json_serializable_default_deserialize_property:
 * @serializable: a #JsonSerializable
 * @property_name: the name of the property
 * @value: a pointer to an uninitialized #GValue
 * @pspec: a #GParamSpec
 * @property_node: a #JsonNode containing the serialized property
 *
 * Calls the default implementation of the #JsonSerializable
 * deserialize_property() virtual function
 *
 * This function can be used inside a custom implementation
 * of the deserialize_property() virtual function in lieu of:
 *
 * |[
 *   JsonSerializable *iface;
 *   gboolean res;
 *
 *   iface = g_type_default_interface_peek (JSON_TYPE_SERIALIZABLE);
 *   res = iface->deserialize_property (serializable, property_name,
 *                                      value,
 *                                      pspec,
 *                                      property_node);
 * ]|
 *
 * Return value: %TRUE if the property was successfully deserialized.
 *
 * Since: 0.10
 */
gboolean
json_serializable_default_deserialize_property (JsonSerializable *serializable,
                                                const gchar      *property_name,
                                                GValue           *value,
                                                GParamSpec       *pspec,
                                                JsonNode         *property_node)
{
  g_return_val_if_fail (JSON_IS_SERIALIZABLE (serializable), FALSE);
  g_return_val_if_fail (property_name != NULL, FALSE);
  g_return_val_if_fail (value != NULL, FALSE);
  g_return_val_if_fail (pspec != NULL, FALSE);
  g_return_val_if_fail (property_node != NULL, FALSE);

  return json_serializable_real_deserialize (serializable,
                                             property_name,
                                             value, pspec,
                                             property_node);
}

/**
 * json_serializable_find_property:
 * @serializable: a #JsonSerializable
 * @name: the name of the property
 *
 * FIXME
 *
 * Return value: (transfer none): the #GParamSpec for the property
 *   or %NULL if no property was found
 *
 * Since: 0.14
 */
GParamSpec *
json_serializable_find_property (JsonSerializable *serializable,
                                 const char       *name)
{
  g_return_val_if_fail (JSON_IS_SERIALIZABLE (serializable), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return JSON_SERIALIZABLE_GET_IFACE (serializable)->find_property (serializable, name);
}

/**
 * json_serializable_list_properties:
 * @serializable: a #JsonSerializable
 * @n_pspecs: (out): return location for the length of the array
 *   of #GParamSpec returned by the function
 *
 * FIXME
 *
 * Return value: (array length=n_pspecs) (transfer container): an array
 *   of #GParamSpec. Use g_free() to free the array when done.
 *
 * Since: 0.14
 */
GParamSpec **
json_serializable_list_properties (JsonSerializable *serializable,
                                   guint            *n_pspecs)
{
  g_return_val_if_fail (JSON_IS_SERIALIZABLE (serializable), NULL);

  return JSON_SERIALIZABLE_GET_IFACE (serializable)->list_properties (serializable, n_pspecs);
}

void
json_serializable_set_property (JsonSerializable *serializable,
                                GParamSpec       *pspec,
                                const GValue     *value)
{
  g_return_if_fail (JSON_IS_SERIALIZABLE (serializable));
  g_return_if_fail (G_IS_PARAM_SPEC (pspec));
  g_return_if_fail (value != NULL);

  JSON_SERIALIZABLE_GET_IFACE (serializable)->set_property (serializable,
                                                            pspec,
                                                            value);
}

void
json_serializable_get_property (JsonSerializable *serializable,
                                GParamSpec       *pspec,
                                GValue           *value)
{
  g_return_if_fail (JSON_IS_SERIALIZABLE (serializable));
  g_return_if_fail (G_IS_PARAM_SPEC (pspec));
  g_return_if_fail (value != NULL);

  JSON_SERIALIZABLE_GET_IFACE (serializable)->get_property (serializable,
                                                            pspec,
                                                            value);
}
