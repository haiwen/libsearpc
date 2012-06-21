#!/usr/bin/python

"""
Generate function define macros.
"""

import string
import sys
import os


# type -> (<c type if used as parameter>, <c type if used as ret type>,
#          <function to get value from array>,
#          <function to set value to the ret_object>,
#          <function to set value to array>,
#          <default_ret_value>)
type_table = {
    "string": ("const gchar*",
               "gchar*", 
               "json_array_get_string_or_null_element",
               "searpc_set_string_to_ret_object",
               "json_array_add_string_or_null_element",
               "NULL"),
    "int": ("int", 
            "int", 
            "json_array_get_int_element",
            "searpc_set_int_to_ret_object",
            "json_array_add_int_element",
            "-1"),
    "int64": ("gint64", 
              "gint64", 
              "json_array_get_int_element",
              "searpc_set_int_to_ret_object",
              "json_array_add_int_element",
              "-1"),
    "object": ("GObject*", 
               "GObject*",
               "",
               "searpc_set_object_to_ret_object",
               "",
               "NULL"),
    "objlist": ("GList*",
                "GList*",
                "",
                "searpc_set_objlist_to_ret_object",
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
    return searpc_marshal_set_ret_common (object, ret_len, error);
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
    f = open('searpc-marshal.h', 'w')
    for item in func_table:
        print >>f, generate_marshal(item[0], item[1])
    f.close()


marshal_register_item = r"""
    {
        searpc_server_register_marshal (${signature_name}(), ${marshal_name});
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
    f = open('searpc-marshal.h', 'a')
    print >>f, "static void register_marshals()"""
    print >>f, "{"
    for item in func_table:
        print >>f, generate_marshal_register_item(item[0], item[1]),
    print >>f, "}"
    f.close()

signature_template = r"""
inline static gchar *
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
    f = open('searpc-signature.h', 'w')
    for item in func_table:
        print >>f,generate_signature(item[0], item[1])
    f.close()


if __name__ == "__main__":
    sys.path.append(os.getcwd())
    gen_marshal_functions()
    gen_marshal_register_function()
    gen_signature_list()
