
inline static const gchar *
searpc_signature_int__string()
{
    return searpc_compute_signature ("int", 1, "string");
}


inline static const gchar *
searpc_signature_int__string_string()
{
    return searpc_compute_signature ("int", 2, "string", "string");
}


inline static const gchar *
searpc_signature_string__void()
{
    return searpc_compute_signature ("string", 0);
}


inline static const gchar *
searpc_signature_string__string()
{
    return searpc_compute_signature ("string", 1, "string");
}


inline static const gchar *
searpc_signature_string__string_int()
{
    return searpc_compute_signature ("string", 2, "string", "int");
}


inline static const gchar *
searpc_signature_objlist__void()
{
    return searpc_compute_signature ("objlist", 0);
}


inline static const gchar *
searpc_signature_objlist__string()
{
    return searpc_compute_signature ("objlist", 1, "string");
}


inline static const gchar *
searpc_signature_objlist__int_int()
{
    return searpc_compute_signature ("objlist", 2, "int", "int");
}


inline static const gchar *
searpc_signature_objlist__string_int()
{
    return searpc_compute_signature ("objlist", 2, "string", "int");
}


inline static const gchar *
searpc_signature_objlist__string_int_int()
{
    return searpc_compute_signature ("objlist", 3, "string", "int", "int");
}


inline static const gchar *
searpc_signature_objlist__string_string_int()
{
    return searpc_compute_signature ("objlist", 3, "string", "string", "int");
}


inline static const gchar *
searpc_signature_object__void()
{
    return searpc_compute_signature ("object", 0);
}


inline static const gchar *
searpc_signature_object__string()
{
    return searpc_compute_signature ("object", 1, "string");
}

