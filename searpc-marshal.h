
static gchar *
marshal_int__string (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);

    int ret = ((int (*)(const gchar*, GError **))func) (param1, &error);

    JsonObject *object = json_object_new ();
    set_int_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_int__string_string (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);
    const gchar* param2 = json_array_get_string_or_null_element (param_array, 2);

    int ret = ((int (*)(const gchar*, const gchar*, GError **))func) (param1, param2, &error);

    JsonObject *object = json_object_new ();
    set_int_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_string__void (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;

    gchar* ret = ((gchar* (*)(GError **))func) (&error);

    JsonObject *object = json_object_new ();
    set_string_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_string__string (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);

    gchar* ret = ((gchar* (*)(const gchar*, GError **))func) (param1, &error);

    JsonObject *object = json_object_new ();
    set_string_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_string__string_int (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);
    int param2 = json_array_get_int_element (param_array, 2);

    gchar* ret = ((gchar* (*)(const gchar*, int, GError **))func) (param1, param2, &error);

    JsonObject *object = json_object_new ();
    set_string_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_objlist__void (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;

    GList* ret = ((GList* (*)(GError **))func) (&error);

    JsonObject *object = json_object_new ();
    set_objlist_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_objlist__string (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);

    GList* ret = ((GList* (*)(const gchar*, GError **))func) (param1, &error);

    JsonObject *object = json_object_new ();
    set_objlist_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_objlist__int_int (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    int param1 = json_array_get_int_element (param_array, 1);
    int param2 = json_array_get_int_element (param_array, 2);

    GList* ret = ((GList* (*)(int, int, GError **))func) (param1, param2, &error);

    JsonObject *object = json_object_new ();
    set_objlist_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_objlist__string_int (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);
    int param2 = json_array_get_int_element (param_array, 2);

    GList* ret = ((GList* (*)(const gchar*, int, GError **))func) (param1, param2, &error);

    JsonObject *object = json_object_new ();
    set_objlist_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_objlist__string_int_int (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);
    int param2 = json_array_get_int_element (param_array, 2);
    int param3 = json_array_get_int_element (param_array, 3);

    GList* ret = ((GList* (*)(const gchar*, int, int, GError **))func) (param1, param2, param3, &error);

    JsonObject *object = json_object_new ();
    set_objlist_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_objlist__string_string_int (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);
    const gchar* param2 = json_array_get_string_or_null_element (param_array, 2);
    int param3 = json_array_get_int_element (param_array, 3);

    GList* ret = ((GList* (*)(const gchar*, const gchar*, int, GError **))func) (param1, param2, param3, &error);

    JsonObject *object = json_object_new ();
    set_objlist_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_object__void (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;

    GObject* ret = ((GObject* (*)(GError **))func) (&error);

    JsonObject *object = json_object_new ();
    set_object_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}


static gchar *
marshal_object__string (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
    const gchar* param1 = json_array_get_string_or_null_element (param_array, 1);

    GObject* ret = ((GObject* (*)(const gchar*, GError **))func) (param1, &error);

    JsonObject *object = json_object_new ();
    set_object_to_ret_object (object, ret);
    return marshal_set_ret_common (object, ret_len, error);
}

static void register_marshals(GHashTable *marshal_table)
{

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_int__string;
        item->signature = searpc_signature_int__string();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_int__string_string;
        item->signature = searpc_signature_int__string_string();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_string__void;
        item->signature = searpc_signature_string__void();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_string__string;
        item->signature = searpc_signature_string__string();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_string__string_int;
        item->signature = searpc_signature_string__string_int();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_objlist__void;
        item->signature = searpc_signature_objlist__void();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_objlist__string;
        item->signature = searpc_signature_objlist__string();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_objlist__int_int;
        item->signature = searpc_signature_objlist__int_int();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_objlist__string_int;
        item->signature = searpc_signature_objlist__string_int();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_objlist__string_int_int;
        item->signature = searpc_signature_objlist__string_int_int();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_objlist__string_string_int;
        item->signature = searpc_signature_objlist__string_string_int();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_object__void;
        item->signature = searpc_signature_object__void();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }

    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = marshal_object__string;
        item->signature = searpc_signature_object__string();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }
}
