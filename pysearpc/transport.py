
class SearpcTransport(object):
    """
    A transport is repsonsible to send the serialized request to the
    server, and get back the raw response from the server.
    """
    def send(self, request_str):
        raise NotImplementedError
