#ifndef SEARPC_NAMED_PIPE_TRANSPORT_H
#define SEARPC_NAMED_PIPE_TRANSPORT_H

#include <pthread.h>
#include <glib.h>
#include <glib-object.h>
#include <jansson.h>

#if defined(WIN32)
#include <windows.h>
#endif

// Implementatin of a searpc transport based on named pipe. It uses unix domain
// sockets on linux/osx, and named pipes on windows.
//
// On the server side, there is a thread that listens for incoming connections,
// and it would create a new thread to handle each connection. Thus the RPC
// functions on the server side may be called from different threads, and it's
// the RPC functions implementation's responsibility to guarantee thread safety
// of the RPC calls. (e.g. using mutexes).

#if defined(WIN32)
typedef HANDLE SearpcNamedPipe;
#else
typedef int SearpcNamedPipe;
#endif

// Server side interface.

typedef struct {
    char path[4096];
    pthread_t listener_thread;
    GList *handlers;
    SearpcNamedPipe pipe_fd;
} SearpcNamedPipeServer;

SearpcNamedPipeServer* searpc_create_named_pipe_server(const char *path);

int searpc_named_pipe_server_start(SearpcNamedPipeServer *server);

// Client side interface.

typedef struct {
    char path[4096];
    SearpcNamedPipe pipe_fd;
} SearpcNamedPipeClient;

SearpcNamedPipeClient* searpc_create_named_pipe_client(const char *path);

SearpcClient * searpc_client_with_named_pipe_transport(SearpcNamedPipeClient *client, const char *service);

int searpc_named_pipe_client_connect(SearpcNamedPipeClient *client);

#endif // SEARPC_NAMED_PIPE_TRANSPORT_H
