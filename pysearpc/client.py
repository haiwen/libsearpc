import fcallfret
import simplejson as json

class SearpcError(Exception):

    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg

class _SearpcObjProps(object):
    '''A compact class to emulate gobject.GProps
    '''
    def __init__(self, dicts):
        new_dict = {}
        for key in dicts:
            value = dicts[key]
            # replace hyphen with with underline
            new_key = key.replace('-', '_')
            new_dict[new_key] = value
            
        self._dicts = new_dict

    def __getattr__(self, key):
        try:
            return self._dicts[key]
        except:
            return None

class _SearpcObj(object):
    '''A compact class to emulate gobject.GObject
    '''
    def __init__(self, dicts):
        self.props = _SearpcObjProps(dicts)


def _fret_obj(ret_str):
    try:
       dicts = json.loads(ret_str)
    except:
        raise SearpcError('Invalid response format')
        
    if dicts.has_key('err_code'):
        raise SearpcError(dicts['err_msg'])
        
    if dicts['ret']:
        return _SearpcObj(dicts['ret'])
    else:
        return None

    
def _fret_objlist(ret_str):
    print ret_str
    try:
       dicts = json.loads(ret_str)
    except:
        raise SearpcError('Invalid response format')
        
    if dicts.has_key('err_code'):
        raise SearpcError(dicts['err_msg'])
        
    l = []
    if dicts['ret']:
        for elt in dicts['ret']:
            l.append(_SearpcObj(elt))
        
    return l

def searpc_func(ret_type, param_types, ret_obj_class=None):
    def decorate(func):
        if len(param_types) == 0:
            fcall = getattr(fcallfret, 'fcall__void')
        else:
            fcall = getattr(fcallfret, 'fcall__' + '_'.join(param_types))
        if ret_type == "void":
            fret = None
        elif ret_type == "object":
            fret = _fret_obj
        elif ret_type == "objlist":
            fret = _fret_objlist
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

            return fret(ret_str)

        if ret_obj_class:
            return newfunc_obj
        else:
            return newfunc

    return decorate


class SearpcClient(object):

    def call_remote_func_sync(self, fcall_str):
        raise NotImplementedError()
