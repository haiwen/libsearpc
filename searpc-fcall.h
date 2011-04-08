
char* searpc_client_fcall__string (const char* fname, const gchar* param1, gsize *len);


char* searpc_client_fcall__string_string (const char* fname, const gchar* param1, const gchar* param2, gsize *len);


char* searpc_client_fcall__void (const char* fname, gsize *len);


char* searpc_client_fcall__string_int (const char* fname, const gchar* param1, int param2, gsize *len);


char* searpc_client_fcall__int_int (const char* fname, int param1, int param2, gsize *len);


char* searpc_client_fcall__string_int_int (const char* fname, const gchar* param1, int param2, int param3, gsize *len);


char* searpc_client_fcall__string_string_int (const char* fname, const gchar* param1, const gchar* param2, int param3, gsize *len);

