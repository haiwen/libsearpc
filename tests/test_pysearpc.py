
import sys
sys.path += ['..', '../pysearpc/.libs']

from pysearpc import SearpcClient, searpc_func, SearpcError
import fcallfret 

class SampleRpcClient(SearpcClient):
    
    def call_remote_func_sync(self, fcall_str):
        return ""

    @searpc_func("void", ["string"])
    def list_peers(self):
        pass

fcallfret.fcall__string("list", "hello")

client = SampleRpcClient()
client.list_peers("10")


