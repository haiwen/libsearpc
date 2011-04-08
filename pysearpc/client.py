
import fcallfret


class SearpcError(Exception):

    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


def searpc_func(ret_type, param_types, ret_obj_class=None):
    """
    ret_obj_class is for ret_type 'object', 'objlist'
    """
    def decorate(func):
        if len(param_types) == 0:
            fcall = getattr(fcallfret, 'fcall__void')
        else:
            fcall = getattr(fcallfret, 'fcall__' + '_'.join(param_types))
        if ret_type == "void":
            fret = None
        else:
            fret = getattr(fcallfret, 'fret__' + ret_type)

        def newfunc(self, *args):
            fcall_str = fcall(func.__name__, *args)
            ret_str = self.call_remote_func_sync(fcall_str)
            if fret:
                try:
                    return fret(ret_str)
                except fcallfret.error, e:
                    raise SearpcError(e)

        def newfunc_obj(self, *args):
            fcall_str = fcall(func.__name__, *args)
            ret_str = self.call_remote_func_sync(fcall_str)
            if fret:
                try:
                    return fret(ret_obj_class, ret_str)
                except fcallfret.error, e:
                    raise SearpcError(e)

        if ret_obj_class:
            return newfunc_obj
        else:
            return newfunc

    return decorate


class SearpcClient(object):

    def call_remote_func_sync(self, fcall_str):
        raise NotImplementedError()
