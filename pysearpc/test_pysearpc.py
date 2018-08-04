#!/usr/bin/env python
#coding: UTF-8

import json
import os
import sys
import unittest
from operator import add, mul

os.chdir(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, '..')
from pysearpc import (
    SearpcClient, SearpcError, SearpcTransport, searpc_func, searpc_server, NamedPipeTransport
)

SVCNAME = 'test-service'

def init_server():
    searpc_server.create_service(SVCNAME)
    searpc_server.register_function(SVCNAME, add, 'add')
    searpc_server.register_function(SVCNAME, mul, 'multi')


class DummyTransport(SearpcTransport):
    def send(self, fcall_str):
        return searpc_server.call_function(SVCNAME, fcall_str)


class SampleRpcClient(SearpcClient):

    def __init__(self):
        self.transport = DummyTransport()

    def call_remote_func_sync(self, fcall_str):
        return self.transport.send(fcall_str)

    @searpc_func("int", ["int", "int"])
    def add(self, x, y):
        pass

    @searpc_func("string", ["string", "int"])
    def multi(self, x, y):
        pass

class SearpcTest(unittest.TestCase):
    def setUp(self):
        init_server()
        self.client = SampleRpcClient()

    def test_normal_transport(self):
        self.run_common()

    @unittest.skip('not implemented yet')
    def test_pipe_transport(self):
        self.client.transport = NamedPipeTransport('/tmp/libsearpc-test.sock')
        self.run_common()

    def run_common(self):
        v = self.client.add(1, 2)
        self.assertEqual(v, 3)

        v = self.client.multi(1, 2)
        self.assertEqual(v, 2)

        v = self.client.multi('abc', 2)
        self.assertEqual(v, 'abcabc')

if __name__ == '__main__':
    unittest.main()
