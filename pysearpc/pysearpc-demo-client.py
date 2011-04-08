
import sys
import socket
from struct import pack, unpack
 
from pysearpc import SearpcClient, searpc_func

SERVER_ADDR = '127.0.0.1'
SERVER_PORT = 12345

def recv_all(sock, length):
    """
    read all n bytes of data from sock
    """

    data = ''
    while len(data) < length:
        more = sock.recv(length - len(data))
        if not more:
            raise EOFError('socket closed %d bytes into a %d-byte message' % (len(data), length))
        data += more
    return data

class SampleRpcClient(SearpcClient):
    
    def call_remote_func_sync(self, fcall_str):
        """
        called by searpc_func to send the request and receive the result
        """
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # connect to server
        s.connect((SERVER_ADDR, SERVER_PORT))
        # send the header
        header = pack('!h', len(fcall_str)); 
        s.sendall(header)
        # send the JSON data
        s.sendall(fcall_str)

        # read the returned header
        header_r = recv_all(s, 2)
        #read the result
        ret_len = list(unpack('!h', header_r))[0]
        if ret_len <= 0:
            raise AssertionError, "returned data length <=  0"

        ret_str = recv_all(s, ret_len)
        return ret_str

    @searpc_func("int", ["string"])
    def searpc_demo_int__string(self):
        pass

client = SampleRpcClient()
res = client.searpc_demo_int__string("hello world")
print 'result from server:', res

