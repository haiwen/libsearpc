import os
import sys
sys.path += ['', '.libs']

RPC_PROC_ID = 1003

SC_CLIENT_CALL = "301"
SS_CLIENT_CALL = "CLIENT CALL"
SC_SERVER_RET = "311"
SS_SERVER_RET = "SERVER RET"


def searpc_transport_send(fcall, priv):

    if priv.peerid is None:
        proc_name = "%s-rpcserver" % (priv.service)
    else:
        proc_name = "remote %s %s-rpcserver" % (priv.peerid, priv.service)

    priv.session.send_request(RPC_PROC_ID, proc_name)
    if priv.session.read_response() < 0:
        return None
    rsp = priv.session.response
    if rsp[0] != "200":
        print "[Sea RPC] failed to start rpc server.\n"
        return None

    priv.session.send_update(RPC_PROC_ID, "301", "CLIENT CALL",
                        fcall, len(fcall))
    if priv.session.read_response() < 0:
        return None
    rsp = priv.session.response
    if rsp[0] == SC_SERVER_RET:
        return rsp[2]
    else:
        print "[Sea RPC] Bad response: %s %s.\n" % (rsp[0], rsp[1])
        return None
