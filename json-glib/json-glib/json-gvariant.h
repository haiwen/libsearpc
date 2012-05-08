/* json-gvariant.h - JSON GVariant integration
 *
 * This file is part of JSON-GLib
 * Copyright (C) 2007  OpenedHand Ltd.
 * Copyright (C) 2009  Intel Corp.
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
 *   Eduardo Lima Mitev  <elima@igalia.com>
 */

#ifndef __JSON_GVARIANT_H__
#define __JSON_GVARIANT_H__

#include <glib.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

JsonNode * json_gvariant_serialize        (GVariant *variant);
gchar *    json_gvariant_serialize_data   (GVariant *variant,
                                           gsize    *length);

GVariant * json_gvariant_deserialize      (JsonNode     *json_node,
                                           const gchar  *signature,
                                           GError      **error);
GVariant * json_gvariant_deserialize_data (const gchar  *json,
                                           gssize        length,
                                           const gchar  *signature,
                                           GError      **error);

G_END_DECLS

#endif /* __JSON_GVARIANT_H__ */
