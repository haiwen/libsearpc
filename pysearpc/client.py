import simplejson as json

class SearpcError(Exception):

    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


def _fret_int(ret_str):
    try:
        dicts = json.loads(ret_str)
    except:
        raise SearpcError('Invalid response format')
        
    if dicts.has_key('err_code'):
        raise SearpcError(dicts['err_msg'])
        
    if dicts['ret']:
        return dicts['ret']

def _fret_string(ret_str):
    try:
        dicts = json.loads(ret_str)
    except:
        raise SearpcError('Invalid response format')
        
    if dicts.has_key('err_code'):
        raise SearpcError(dicts['err_msg'])
        
    if dicts['ret']:
        return dicts['ret']

class _SearpcObj(object):
    '''A compact class to emulate gobject.GObject
    '''
    def __init__(self, dicts):
        new_dict = {}
        for key in dicts:
            value = dicts[key]
            # replace hyphen with with underline
            new_key = key.replace('-', '_')
            new_dict[new_key] = value
        # For compatibility with old usage peer.props.name
        self.props = self
        self._dict = new_dict

    def __getattr__(self, key):
        try:
            return self._dict[key]
        except:
            return None

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


def searpc_func(ret_type, param_types):

    def decorate(func):
        if ret_type == "void":
            fret = None
        elif ret_type == "object":
            fret = _fret_obj
        elif ret_type == "objlist":
            fret = _fret_objlist
        elif ret_type == "int":
            fret = _fret_int
        elif ret_type == "int64":
            fret = _fret_int
        elif ret_type == "string":
            fret = _fret_string
        else:
            raise SearpcError('Invial return type')

        def newfunc(self, *args):
            array = [func.__name__] + list(args)
            fcall_str = json.dumps(array)
            ret_str = self.call_remote_func_sync(fcall_str)
            if fret:
                return fret(ret_str)

        return newfunc

    return decorate


class SearpcClient(object):

    def call_remote_func_sync(self, fcall_str):
        raise NotImplementedError()
