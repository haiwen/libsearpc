
class SearpcTransport(object):
    """
    A transport is repsonsible to send the serialized request to the
    server, and get back the raw response from the server.
    """
    def send(self, request_str):
        raise NotImplementedError


class NamedPipeTransport(SearpcTransport):
    def __init__(self, pipe_path):
        self.pipe_path = pipe_path

    def send(self, fcall_str):
        pass
