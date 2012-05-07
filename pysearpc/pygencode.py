#!/usr/bin/python


import string
import sys

sys.path += ['../']

module_func_array_template = r"""
static PyMethodDef SearpcClientModule_Functions[] = {
    ${array}
    {NULL, NULL, 0, NULL},
};
"""

func_item_template = r"""
    {"${pyfuncname}", (PyCFunction)${cfuncname},
     METH_VARARGS, "" },"""


def gen_fcall_funcs_array(arg_types):

    fcall_array = ""

    if len(arg_types) == 0:
        pyfuncname = "fcall__void"
        cfuncname = "SearpcClient_Fcall__Void"
    else:
        pyfuncname = "fcall__" + "_".join(arg_types)
        tmplist = []
        for arg in arg_types:
            tmplist.append(string.capitalize(arg))

        cfuncname = "SearpcClient_Fcall__" + "_".join(tmplist)

    return string.Template(func_item_template)\
            .substitute(pyfuncname=pyfuncname,
                        cfuncname=cfuncname)


def gen_fret_funcs_array(ret_type):

    fret_array = ""

    if ret_type is None:
        pyfuncname = "fret__void"
        cfuncname = "SearpcClient_Fret__Void"
    else:
        pyfuncname = "fret__" + ret_type
        cfuncname = "SearpcClient_Fret__" + string.capitalize(ret_type)

    return string.Template(func_item_template)\
            .substitute(pyfuncname=pyfuncname,
                        cfuncname=cfuncname)


def gen_module_funcs_array():
    """
    Generate static PyMethodDef SearpcClientModule_Functions[]
    """
    from rpc_table import func_table

    # generate fcall methods array
    fcall_array = ""
    arg_types_list = []
    for item in func_table:
        if item[1] not in arg_types_list:
            arg_types_list.append(item[1])

    for item in arg_types_list:
        fcall_array += gen_fcall_funcs_array(item)

    # generate fret methods array
    fret_array = ""
    ret_types_list = ["int", "int64", "string"]
    for item in ret_types_list:
        fret_array += gen_fret_funcs_array(item)

    array = fcall_array
    array += fret_array

    print string.Template(module_func_array_template)\
            .substitute(array=array)


type_table = {
    "string" : ("char *", "z"),
    "int" : ("int", "i"),
    "int64" : ("gint64", "L"),
    "object" : ("GObject *", "O"),
}


fcall_template = r"""
static PyObject *
SearpcClient_Fcall__${Suffix}(PyObject *self,
                              PyObject *args)
{
    char *fname;
${def_args}
    char *fcall;
    gsize len;

    if (!PyArg_ParseTuple(args, "${fmt}", ${args_addr}))
        return NULL;

    fcall = searpc_client_fcall__${suffix}(${args});

    return PyString_FromString(fcall);
}
"""


def gen_fcall_func(arg_types):

    if len(arg_types) == 0:
        Suffix = "Void"
        suffix = "void"
        def_args = ""
        kwargs_str = "NULL"
        fmt = "s"
        args_addr = "&fname"
        args = "fname, &len"
        return string.Template(fcall_template)\
                .substitute(Suffix=Suffix, suffix=suffix,
                            def_args=def_args, kwargs_str=kwargs_str,
                            fmt=fmt, args_addr=args_addr, args=args)

    tmplist = []
    for arg in arg_types:
        tmplist.append(string.capitalize(arg))
    Suffix = "_".join(tmplist)
    suffix = "_".join(arg_types)
    def_args = ""
    kwargs_str = ""
    fmt = "s"
    args_addr = "&fname"
    args = "fname"
    for i, arg_type in enumerate(arg_types):
        def_args += "    " + type_table[arg_type][0] + " param" + \
                    str(i + 1) + ";\n"
        kwargs_str += '"param' + str(i + 1) + '", '
        fmt += type_table[arg_type][1]
        args_addr += ", &param" + str(i + 1)
        args += ", param" + str(i + 1)
    kwargs_str += "NULL"
    args += ", &len"
    return string.Template(fcall_template)\
            .substitute(Suffix=Suffix, suffix=suffix,
                        def_args=def_args, kwargs_str=kwargs_str,
                        fmt=fmt, args_addr=args_addr, args=args)


def gen_fcall_list():

    from rpc_table import func_table

    arg_types_list = []
    for item in func_table:
        if item[1] not in arg_types_list:
            arg_types_list.append(item[1])

    for item in arg_types_list:
        print gen_fcall_func(item)


if __name__ == "__main__":
    gen_fcall_list()
    gen_module_funcs_array()

