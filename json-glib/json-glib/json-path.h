/* json-path.h - JSONPath implementation
 *
 * This file is part of JSON-GLib
 * Copyright © 2011  Intel Corp.
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

#if !defined(__JSON_GLIB_INSIDE__) && !defined(JSON_COMPILATION)
#error "Only <json-glib/json-glib.h> can be included directly."
#endif

#ifndef __JSON_PATH_H__
#define __JSON_PATH_H__

#include <json-glib/json-types.h>

G_BEGIN_DECLS

#define JSON_TYPE_PATH          (json_path_get_type ())
#define JSON_PATH(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), JSON_TYPE_PATH, JsonPath))
#define JSON_IS_PATH(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JSON_TYPE_PATH))

/**
 * JSON_PATH_ERROR:
 *
 * Error domain for #JsonPath errors
 *
 * Since: 0.14
 */
#define JSON_PATH_ERROR         (json_path_error_quark ())

/**
 * JsonPathError:
 * @JSON_PATH_ERROR_INVALID_QUERY: Invalid query
 *
 * Error code enumeration for the %JSON_PATH_ERROR domain.
 *
 * Since: 0.14
 */
typedef enum {
  JSON_PATH_ERROR_INVALID_QUERY
} JsonPathError;

/**
 * JsonPath:
 *
 * The <structname>JsonPath</structname> structure is an opaque object
 * whose members cannot be directly accessed except through the provided
 * API.
 *
 * Since: 0.14
 */
typedef struct _JsonPath        JsonPath;

/**
 * JsonPathClass:
 *
 * The <structname>JsonPathClass</structname> structure is an opaque
 * object class whose members cannot be directly accessed.
 *
 * Since: 0.14
 */
typedef struct _JsonPathClass   JsonPathClass;

GType json_path_get_type (void) G_GNUC_CONST;
GQuark json_path_error_quark (void);

JsonPath *      json_path_new           (void);

gboolean        json_path_compile       (JsonPath    *path,
                                         const char  *expression,
                                         GError     **error);
JsonNode *      json_path_match         (JsonPath    *path,
                                         JsonNode    *root);

JsonNode *      json_path_query         (const char  *expression,
                                         JsonNode    *root,
                                         GError     **error);

G_END_DECLS

#endif /* __JSON_PATH_H__ */
