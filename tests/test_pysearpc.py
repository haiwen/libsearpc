
import sys
sys.path += ['..']

from pysearpc import SearpcClient, searpc_func, SearpcError

class SampleRpcClient(SearpcClient):
    
    def call_remote_func_sync(self, fcall_str):
        return ""

    @searpc_func("void", ["string", "int"])
    def list_peers(self):
        pass

client = SampleRpcClient()
client.list_peers("id", 10)
