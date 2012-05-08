#ifndef SEARPC_UTILS_H
#define SEARPC_UTILS_H

#include "json-glib/json-glib.h"


inline static const gchar *
json_object_get_string_or_null_member (JsonObject *object, const gchar *member_name)
{
    if (!json_object_get_null_member (object, member_name))
        return json_object_get_string_member (object, member_name);
    else
        return NULL;
}

inline static void
json_object_set_string_or_null_member (JsonObject *object,
                                       const gchar *member_name,
                                       const gchar *value)
{
    if (value)
        json_object_set_string_member (object, member_name, value);
    else
        json_object_set_null_member (object, member_name);
}

inline static const gchar *
json_array_get_string_or_null_element (JsonArray *array, guint index)
{
    if (!json_array_get_null_element (array, index))
        return json_array_get_string_element (array, index);
    else
        return NULL;
}

inline static void
json_array_add_string_or_null_element (JsonArray *array, const gchar *value)
{
    if (value)
        json_array_add_string_element (array, value);
    else
        json_array_add_null_element (array);
}



#endif
