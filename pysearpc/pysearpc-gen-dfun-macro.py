#!/usr/bin/python

"""
Generate function define macros.
"""

import string
import sys

sys.path += ['../']


# XXX: generate searpc-client.h file
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
    ret_types_list = []
    for item in func_table:
        if item[0] not in ret_types_list:
            ret_types_list.append(item[0])

    for item in ret_types_list:
        fret_array += gen_fret_funcs_array(item)

    array = fcall_array
    array += fret_array

    print string.Template(module_func_array_template)\
            .substitute(array=array)


type_table = {
    "string" : ("char *", "s"),
    "int" : ("int", "i"),
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


ret_table = {
    "string" : "PyString_FromString",
    "int" : "PyLong_FromLong",
    "object" : "pygobject_new",
}


fret_template = r"""
static PyObject *
SearpcClient_Fret__${Suffix}(PyObject *self,
                             PyObject *args)
{
    char *data;
${def_args}
    GError *error = NULL;

    if (!PyArg_ParseTuple(args, "${fmt}", ${args_addr}))
        return NULL;

    res = searpc_client_fret__${suffix}(${args}, strlen(data), &error);

    return ${ret_func};
}
"""

fret_objlist_template = r"""
static PyObject *SearpcClient_Fret__Objlist(PyObject *self,
                                            PyObject *args)
{
    char *data;
    GType type;
    PyObject *type_obj, *res, *tmp;
    GList *list, *p;
    GError *error = NULL;
    static char *kwlist[] = { "type", "data", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Os",
         kwlist, &type_obj, &data))
            return NULL;

    type = pyg_type_from_object(type_obj);
    list = searpc_client_fret__objlist(type, data, strlen(data), &error);

    res = PyList_New(0);
    for (p = list; p; p = p->next) {
        tmp = pygobject_new(p->data);
        PyList_Append(res, tmp);
    }

    return res;
}
"""


def gen_fret_func(ret_type):

    if ret_type == "objlist":
        return fret_objlist_template

    Suffix = string.capitalize(ret_type)
    suffix = ret_type
    def_args = ""
    kwargs_str = ""
    fmt = ""
    args_addr = ""
    args = ""
    if ret_type == "object":
        def_args += "    GType type;\n"
        def_args += "    PyObject *type_obj;\n"
        def_args += "    GObject *res;"
        kwargs_str = '"type", "data", NULL'
        fmt = "Os"
        args_addr = "&type_obj, &data"
        args = "type, data"
    else:
        def_args += "    " + type_table[ret_type][0] + " res;"
        kwargs_str = '"data", NULL'
        fmt = "s"
        args_addr = "&data"
        args = "data"
        
    ret_func = ret_table[ret_type] + "(res);"

    return string.Template(fret_template)\
            .substitute(Suffix=Suffix, suffix=suffix,
                        def_args=def_args, kwargs_str=kwargs_str,
                        fmt=fmt, args_addr=args_addr, args=args,
                        ret_func=ret_func)


def gen_fret_list():

    from rpc_table import func_table

    ret_type_list = []
    for item in func_table:
        if item[0] not in ret_type_list:
            ret_type_list.append(item[0])

    for item in ret_type_list:
        print gen_fret_func(item)


# XXX: generate searpc.py file
searpc_file_hdr_template = r"""
import os
import sys
sys.path += ['', '.libs']
import json
import new
import types

from searpc_client import SearpcClientBase
"""

searpc_client_template = r"""
class SearpcClient(SearpcClientBase):
    pass

def searpc_client_transport_send(client, fcall):
    return client.transport_send(fcall, client.priv)
"""

searpc_defun_template = r"""
def defun_${ret_type}__${args_type}(func):
    def newfunc(${args1}):
        fcall = self.fcall__${args_type}(${args2})
        fret = searpc_client_transport_send(self, fcall)
        if fret is None:
            return None
        return self.fret__${ret_type}(${obj_type}fret)
    return newfunc
"""

def gen_decorator(ret_type, args_type_list):

    if len(args_type_list) == 0:
        args_type = "void"
        args1 = "self"
        args2 = "func.__name__"

        return string.Template(searpc_defun_template)\
                .substitute(ret_type=ret_type, args_type=args_type,
                            args1=args1, args2=args2, obj_type="")

    args_type = "_".join(args_type_list)
    args1 = "self"
    args2 = "func.__name__"
    obj_type = ""
    if ret_type == "object" or ret_type == "objlist":
        args1 += ", obj_type"
        obj_type = "obj_type, "
    for i, arg_type in enumerate(args_type_list):
        args1 += ", param" + str(i + 1)
        args2 += ", param" + str(i + 1)

    return string.Template(searpc_defun_template)\
            .substitute(ret_type=ret_type, args_type=args_type,
                        args1=args1, args2=args2, obj_type=obj_type)


def gen_func_decorator():

    from rpc_table import func_table

    for item in func_table:
        print gen_decorator(item[0], item[1])


def gen_pysearpc():
    print searpc_file_hdr_template
    print searpc_client_template
    gen_func_decorator()


if __name__ == "__main__":
    cmd = sys.argv[1]

    if cmd == "gen-searpcclient":
        gen_fcall_list()
        gen_module_funcs_array()
    elif cmd == "gen-pysearpc":
        gen_pysearpc()
    else:
        print "arg error"
