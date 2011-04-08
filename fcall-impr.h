
char*
searpc_client_fcall__string (const char* fname, const gchar* param1, gsize *len)
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);
    json_array_add_string_or_null_element (array, param1);

    return fcall_common(array, len);
}


char*
searpc_client_fcall__string_string (const char* fname, const gchar* param1, const gchar* param2, gsize *len)
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);
    json_array_add_string_or_null_element (array, param1);
    json_array_add_string_or_null_element (array, param2);

    return fcall_common(array, len);
}


char*
searpc_client_fcall__void (const char* fname, gsize *len)
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);

    return fcall_common(array, len);
}


char*
searpc_client_fcall__string_int (const char* fname, const gchar* param1, int param2, gsize *len)
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);
    json_array_add_string_or_null_element (array, param1);
    json_array_add_int_element (array, param2);

    return fcall_common(array, len);
}


char*
searpc_client_fcall__int_int (const char* fname, int param1, int param2, gsize *len)
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);
    json_array_add_int_element (array, param1);
    json_array_add_int_element (array, param2);

    return fcall_common(array, len);
}


char*
searpc_client_fcall__string_int_int (const char* fname, const gchar* param1, int param2, int param3, gsize *len)
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);
    json_array_add_string_or_null_element (array, param1);
    json_array_add_int_element (array, param2);
    json_array_add_int_element (array, param3);

    return fcall_common(array, len);
}


char*
searpc_client_fcall__string_string_int (const char* fname, const gchar* param1, const gchar* param2, int param3, gsize *len)
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);
    json_array_add_string_or_null_element (array, param1);
    json_array_add_string_or_null_element (array, param2);
    json_array_add_int_element (array, param3);

    return fcall_common(array, len);
}

