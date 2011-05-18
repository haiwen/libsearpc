#!/usr/bin/python

"""
Generate function define macros.
"""

import string
import sys


# type -> (<c type if used as parameter>, <c type if used as ret type>,
#          <function to get value from array>,
#          <function to set value to the ret_object>,
#          <function to set value to array>,
#          <default_ret_value>)
type_table = {
    "string": ("const gchar*",
               "gchar*", 
               "json_array_get_string_or_null_element",
               "set_string_to_ret_object",
               "json_array_add_string_or_null_element",
               "NULL"),
    "int": ("int", 
            "int", 
            "json_array_get_int_element",
            "set_int_to_ret_object",
            "json_array_add_int_element",
            "-1"),
    "object": ("GObject*", 
               "GObject*",
               "",
               "set_object_to_ret_object",
               "",
               "NULL"),
    "objlist": ("GList*",
                "GList*",
                "",
                "set_objlist_to_ret_object",
                "",
                "NULL"),
}

marshal_template = r"""
static gchar *
${marshal_name} (void *func, JsonArray *param_array, gsize *ret_len)
{
    GError *error = NULL;
${get_parameters}
    ${func_call}

    JsonObject *object = json_object_new ();
    ${convert_ret}
    return marshal_set_ret_common (object, ret_len, error);
}
"""

def generate_marshal(ret_type, arg_types):
    ret_type_item = type_table[ret_type]
    ret_type_in_c = ret_type_item[1]

    template = string.Template(marshal_template)

    if len(arg_types) == 0:
        marshal_name = "marshal_" + ret_type + "__void"
    else:
        marshal_name = "marshal_" + ret_type + "__" + ('_'.join(arg_types))
    get_parameters = ""
    for i, arg_type in enumerate(arg_types):
        type_item = type_table[arg_type]
        stmt = "    %s param%d = %s (param_array, %d);\n" %(
            type_item[0], i+1, type_item[2], i+1)
        get_parameters += stmt
    
    # func_prototype should be something like 
    # GList* (*)(const char*, int, GError **)
    func_prototype = ret_type_in_c + " (*)("
    for arg_type in arg_types:
        func_prototype += type_table[arg_type][0] + ", "
    func_prototype += "GError **)"

    func_args = ""
    for i in range(1, len(arg_types)+1):
        func_args += "param%d, " % (i)
    func_args += "&error"

    func_call = "%s ret = ((%s)func) (%s);" % (ret_type_in_c, func_prototype,
                                              func_args)
    
    convert_ret = "%s (object, ret);" % ret_type_item[3]
    
    return template.substitute(marshal_name=marshal_name,
                               get_parameters=get_parameters,
                               func_call=func_call,
                               convert_ret=convert_ret)

def gen_marshal_functions():
    from rpc_table import func_table
    for item in func_table:
        print generate_marshal(item[0], item[1])


marshal_register_item = r"""
    {
        MarshalItem *item = g_new0(MarshalItem, 1);
        item->mfunc = ${marshal_name};
        item->signature = ${signature_name}();
        g_hash_table_insert (marshal_table, (gpointer)item->signature, item);
    }
"""

def generate_marshal_register_item(ret_type, arg_types):
    if len(arg_types) == 0:
        marshal_name = "marshal_" + ret_type + "__void"
    else:
        marshal_name = "marshal_" + ret_type + "__" + ('_'.join(arg_types))

    if len(arg_types) == 0:
        signature_name = "searpc_signature_" + ret_type + "__void"
    else:
        signature_name = "searpc_signature_" + ret_type + "__" + (
            '_'.join(arg_types))

    return string.Template(marshal_register_item).substitute(
        marshal_name=marshal_name,
        signature_name=signature_name)

def gen_marshal_register_function():
    from rpc_table import func_table
    print "static void register_marshals(GHashTable *marshal_table)"""
    print "{"
    for item in func_table:
        print generate_marshal_register_item(item[0], item[1]),
    print "}"

signature_template = r"""
inline static const gchar *
${signature_name}()
{
    return searpc_compute_signature (${args});
}
"""

def generate_signature(ret_type, arg_types):
    ret_type_item = type_table[ret_type]
    ret_type_in_c = ret_type_item[1]

    if len(arg_types) == 0:
        signature_name = "searpc_signature_" + ret_type + "__void"
    else:
        signature_name = "searpc_signature_" + ret_type + "__" + (
            '_'.join(arg_types))
    
    args = "\"" + ret_type + "\"" + ", " + str(len(arg_types))
    for arg_type in arg_types:
        args += ", " + "\"" + arg_type + "\""
    
    template = string.Template(signature_template)
    return template.substitute(signature_name=signature_name, args=args)

def gen_signature_list():
    from rpc_table import func_table
    for item in func_table:
        print generate_signature(item[0], item[1])


fcall_template = r"""
char*
searpc_client_fcall__${suffix} (const char* fname, ${args})
{
    JsonArray *array;

    g_return_val_if_fail (fname != NULL, NULL);

    array = json_array_new ();
    json_array_add_string_element (array, fname);
${args_to_array}
    return fcall_common(array, len);
}
"""

def gen_fcall(arg_types):
    if len(arg_types) == 0:
        suffix = "void"
        args = "gsize *len"
        args_to_array = ""
        return string.Template(fcall_template).substitute(suffix=suffix,
                             args=args, args_to_array=args_to_array)

    suffix = "_".join(arg_types)
    args = ""
    args_to_array = ""
    for i, arg_type in enumerate(arg_types):
        args += type_table[arg_type][0] + " param" + str(i+1) + ", "
        args_to_array += "    " + type_table[arg_type][4] + " (array, param" + str(i+1) +");\n"
    args += "gsize *len"
    return string.Template(fcall_template).substitute(suffix=suffix,
                             args=args, args_to_array=args_to_array)    

def gen_fcall_list():
    from rpc_table import func_table

    arg_types_list = []
    for item in func_table:
        if item[1] not in arg_types_list:
            arg_types_list.append(item[1])

    for item in arg_types_list:
        print gen_fcall(item)

fcall_declare_template = r"""
char* searpc_client_fcall__${suffix} (const char* fname, ${args});
"""

def gen_fcall_declare(arg_types):
    if len(arg_types) == 0:
        suffix = "void"
        args = "gsize *len"
        return string.Template(fcall_declare_template).substitute(suffix=suffix,
                                                                  args=args)

    suffix = "_".join(arg_types)
    args = ""
    for i, arg_type in enumerate(arg_types):
        args += type_table[arg_type][0] + " param" + str(i+1) + ", "
    args += "gsize *len"
    return string.Template(fcall_declare_template).substitute(suffix=suffix,
                                                              args=args)

def gen_fcall_declare_list():
    from rpc_table import func_table

    arg_types_list = []
    for item in func_table:
        if item[1] not in arg_types_list:
            arg_types_list.append(item[1])

    for item in arg_types_list:
        print gen_fcall_declare(item)


dfun_template_with_gtype = r"""
#define SEARPC_CLIENT_DEFUN_${RET_TYPE}__${ARG_TYPES}(funcname, gtype)        \
${ret_type_in_c}                                                              \
funcname (SearpcClient *client, ${args})                                \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    ${ret_type_in_c} result;                                            \
                                                                        \
    fcall = searpc_client_fcall__${arg_types} (#funcname,                 \
                                            ${fcall_args});             \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return ${default_value};                                        \
    }                                                                   \
    result = searpc_client_fret__${ret_type} (gtype, fret, ret_len, error); \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}
"""

dfun_template_without_gtype = r"""
#define SEARPC_CLIENT_DEFUN_${RET_TYPE}__${ARG_TYPES}(funcname)         \
${ret_type_in_c}                                                        \
funcname (SearpcClient *client, ${args}) \
{                                                                       \
    char *fcall, *fret;                                                 \
    size_t fcall_len, ret_len;                                          \
    ${ret_type_in_c} result;                                            \
                                                                        \
    fcall = searpc_client_fcall__${arg_types} (#funcname,               \
                                               ${fcall_args});          \
    fret = searpc_client_transport_send (client,                        \
                                         fcall,                         \
                                         fcall_len,                     \
                                         &ret_len);                     \
    if (!fret) {                                                        \
        g_free (fcall);                                                 \
        g_set_error (error, 0, TRANSPORT_ERROR_CODE, TRANSPORT_ERROR);  \
        return ${default_value};                                        \
    }                                                                   \
    result = searpc_client_fret__${ret_type} (fret, ret_len, error);    \
                                                                        \
    g_free (fcall);                                                     \
    g_free (fret);                                                      \
    return result;                                                      \
}
"""

def gen_dfun_macro(ret_type, arg_types):
    if ret_type in ['object', 'objlist']:
        template = string.Template(dfun_template_with_gtype)
    else:
        template = string.Template(dfun_template_without_gtype)

    if len(arg_types) == 0:
        arg_types_str = "void"
    else:
        arg_types_str = "_".join(arg_types)

    args = ""
    for i, arg_type in enumerate(arg_types):
        args += type_table[arg_type][0] + " param" + str(i+1) + ", "
    args += "GError **error"

    fcall_args = ""
    for i, arg_type in enumerate(arg_types):
        fcall_args += " param" + str(i+1) + ", "
    fcall_args += "&fcall_len"

    default_value = type_table[ret_type][5]
    ret_type_in_c = type_table[ret_type][1]
    
    return template.substitute(ret_type=ret_type, RET_TYPE=ret_type.upper(),
                               ret_type_in_c=ret_type_in_c,
                        arg_types=arg_types_str, ARG_TYPES=arg_types_str.upper(),
                        args=args, fcall_args=fcall_args,
                        default_value=default_value)

def gen_dfun_macro_list():
    from rpc_table import func_table
    for item in func_table:
        print gen_dfun_macro(item[0], item[1])

if __name__ == "__main__":
    command = sys.argv[1]
    if command == "gen-marshal":
        gen_marshal_functions()
        gen_marshal_register_function()
    elif command == "gen-signature":
        gen_signature_list()
    elif command == "gen-fcall":
        gen_fcall_list()
    elif command == "gen-fcall-declare":
        gen_fcall_declare_list()
    elif command == "gen-dfun-macro":
        gen_dfun_macro_list()
    else:
        print "Unknown command %s" % (command) 
