#!/usr/bin/env python
#coding: UTF-8

import json
import sys
import unittest
from operator import add

sys.path.insert(0, '..')
from pysearpc import (
    SearpcClient, SearpcError, SearpcTransport, searpc_func, searpc_server
)

SVCNAME = 'test-service'

def multi(a, b):
    return a * b

def init_server():
    searpc_server.create_service(SVCNAME)
    searpc_server.register_function(SVCNAME, add, 'add')
    searpc_server.register_function(SVCNAME, multi, 'multi')


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

class SearpcTest(unittest.TestCase):
    def setUp(self):
        init_server()
        self.client = SampleRpcClient()

    def testNormalTransport(self):
        v = self.client.add(1, 2)
        self.assertEqual(v, 3)

if __name__ == '__main__':
    unittest.main()
