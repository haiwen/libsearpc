/* json-builder.h: JSON tree builder
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

#if !defined(__JSON_GLIB_INSIDE__) && !defined(JSON_COMPILATION)
#error "Only <json-glib/json-glib.h> can be included directly."
#endif

#ifndef __JSON_BUILDER_H__
#define __JSON_BUILDER_H__

#include <json-glib/json-types.h>

G_BEGIN_DECLS

#define JSON_TYPE_BUILDER             (json_builder_get_type ())
#define JSON_BUILDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), JSON_TYPE_BUILDER, JsonBuilder))
#define JSON_IS_BUILDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JSON_TYPE_BUILDER))
#define JSON_BUILDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), JSON_TYPE_BUILDER, JsonBuilderClass))
#define JSON_IS_BUILDER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), JSON_TYPE_BUILDER))
#define JSON_BUILDER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), JSON_TYPE_BUILDER, JsonBuilderClass))

typedef struct _JsonBuilder           JsonBuilder;
typedef struct _JsonBuilderPrivate    JsonBuilderPrivate;
typedef struct _JsonBuilderClass      JsonBuilderClass;

/**
 * JsonBuilder:
 *
 * The <structname>JsonBuilder</structname> structure contains only
 * private data and shouls be accessed using the provided API
 *
 * Since: 0.12
 */
struct _JsonBuilder
{
  /*< private >*/
  GObject parent_instance;

  JsonBuilderPrivate *priv;
};

/**
 * JsonBuilderClass:
 *
 * The <structname>JsonBuilder</structname> structure contains only
 * private data
 *
 * Since: 0.12
 */
struct _JsonBuilderClass
{
  /*< private >*/
  GObjectClass parent_class;

  /* padding, for future expansion */
  void (* _json_reserved1) (void);
  void (* _json_reserved2) (void);
};

GType json_builder_get_type (void) G_GNUC_CONST;

JsonBuilder *json_builder_new                (void);
JsonNode    *json_builder_get_root           (JsonBuilder  *builder);
void         json_builder_reset              (JsonBuilder  *builder);

JsonBuilder *json_builder_begin_array        (JsonBuilder  *builder);
JsonBuilder *json_builder_end_array          (JsonBuilder  *builder);
JsonBuilder *json_builder_begin_object       (JsonBuilder  *builder);
JsonBuilder *json_builder_end_object         (JsonBuilder  *builder);

JsonBuilder *json_builder_set_member_name    (JsonBuilder  *builder,
                                              const gchar  *member_name);
JsonBuilder *json_builder_add_value          (JsonBuilder  *builder,
                                              JsonNode     *node);
JsonBuilder *json_builder_add_int_value      (JsonBuilder  *builder,
                                              gint64        value);
JsonBuilder *json_builder_add_double_value   (JsonBuilder  *builder,
                                              gdouble       value);
JsonBuilder *json_builder_add_boolean_value  (JsonBuilder  *builder,
                                              gboolean      value);
JsonBuilder *json_builder_add_string_value   (JsonBuilder  *builder,
                                              const gchar  *value);
JsonBuilder *json_builder_add_null_value     (JsonBuilder  *builder);

G_END_DECLS

#endif /* __JSON_BUILDER_H__ */
