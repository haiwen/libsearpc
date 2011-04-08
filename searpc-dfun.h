
#define SEARPC_CLIENT_DEFUN_INT__STRING(funcname)         \
int                                                        \
funcname (SearpcClient *client, const gchar* param1, GError **error) \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    int result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string (#funcname,               \
                                                param1, &fcall_len);          \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return -1;                                        \
    }                                                                   \
    result = searpc_client_fret__int (fret, ret_len, error);    \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_INT__STRING_STRING(funcname)         \
int                                                        \
funcname (SearpcClient *client, const gchar* param1, const gchar* param2, GError **error) \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    int result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string_string (#funcname,               \
                                                param1,  param2, &fcall_len);          \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return -1;                                        \
    }                                                                   \
    result = searpc_client_fret__int (fret, ret_len, error);    \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_STRING__VOID(funcname)         \
gchar*                                                        \
funcname (SearpcClient *client, GError **error) \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    gchar* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__void (#funcname,               \
                                               &fcall_len);          \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__string (fret, ret_len, error);    \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_STRING__STRING(funcname)         \
gchar*                                                        \
funcname (SearpcClient *client, const gchar* param1, GError **error) \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    gchar* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string (#funcname,               \
                                                param1, &fcall_len);          \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__string (fret, ret_len, error);    \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_STRING__STRING_INT(funcname)         \
gchar*                                                        \
funcname (SearpcClient *client, const gchar* param1, int param2, GError **error) \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    gchar* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string_int (#funcname,               \
                                                param1,  param2, &fcall_len);          \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__string (fret, ret_len, error);    \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJLIST__VOID(funcname, gtype)        \
GList*                                                              \
funcname (SearpcClient *client, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GList* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__void (#funcname,                 \
                                            &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__objlist (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJLIST__STRING(funcname, gtype)        \
GList*                                                              \
funcname (SearpcClient *client, const gchar* param1, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GList* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string (#funcname,                 \
                                             param1, &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__objlist (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJLIST__INT_INT(funcname, gtype)        \
GList*                                                              \
funcname (SearpcClient *client, int param1, int param2, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GList* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__int_int (#funcname,                 \
                                             param1,  param2, &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__objlist (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJLIST__STRING_INT(funcname, gtype)        \
GList*                                                              \
funcname (SearpcClient *client, const gchar* param1, int param2, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GList* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string_int (#funcname,                 \
                                             param1,  param2, &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__objlist (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJLIST__STRING_INT_INT(funcname, gtype)        \
GList*                                                              \
funcname (SearpcClient *client, const gchar* param1, int param2, int param3, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GList* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string_int_int (#funcname,                 \
                                             param1,  param2,  param3, &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__objlist (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJLIST__STRING_STRING_INT(funcname, gtype)        \
GList*                                                              \
funcname (SearpcClient *client, const gchar* param1, const gchar* param2, int param3, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GList* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string_string_int (#funcname,                 \
                                             param1,  param2,  param3, &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__objlist (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJECT__VOID(funcname, gtype)        \
GObject*                                                              \
funcname (SearpcClient *client, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GObject* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__void (#funcname,                 \
                                            &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__object (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}


#define SEARPC_CLIENT_DEFUN_OBJECT__STRING(funcname, gtype)        \
GObject*                                                              \
funcname (SearpcClient *client, const gchar* param1, GError **error)                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    GObject* result;                                            \
                                                                        \
    fcall = searpc_client_fcall__string (#funcname,                 \
                                             param1, &fcall_len);             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return NULL;                                        \
    }                                                                   \
    result = searpc_client_fret__object (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}

