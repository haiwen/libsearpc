#ifndef SEARPC_NAMED_PIPE_TRANSPORT_H
#define SEARPC_NAMED_PIPE_TRANSPORT_H

#ifdef LIBSEARPC_EXPORTS
#define LIBSEARPC_API __declspec(dllexport)
#else
#define LIBSEARPC_API
#endif

#include <pthread.h>
#include <glib.h>
#include <glib-object.h>
#include <jansson.h>

#if defined(WIN32)
#include <windows.h>
#endif

G_BEGIN_DECLS

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

struct _SearpcNamedPipeServer {
    char path[4096];
    pthread_t listener_thread;
    SearpcNamedPipe pipe_fd;
    GThreadPool *named_pipe_server_thread_pool;
};

typedef struct _SearpcNamedPipeServer LIBSEARPC_API SearpcNamedPipeServer;

LIBSEARPC_API
SearpcNamedPipeServer* searpc_create_named_pipe_server(const char *path);

LIBSEARPC_API
SearpcNamedPipeServer* searpc_create_named_pipe_server_with_threadpool(const char *path, int named_pipe_server_thread_pool_size);

LIBSEARPC_API
int searpc_named_pipe_server_start(SearpcNamedPipeServer *server);

// Client side interface.

struct _SearpcNamedPipeClient {
    char path[4096];
    SearpcNamedPipe pipe_fd;
};

typedef struct _SearpcNamedPipeClient LIBSEARPC_API SearpcNamedPipeClient;

LIBSEARPC_API
SearpcNamedPipeClient* searpc_create_named_pipe_client(const char *path);

LIBSEARPC_API
SearpcClient * searpc_client_with_named_pipe_transport(SearpcNamedPipeClient *client, const char *service);

LIBSEARPC_API
int searpc_named_pipe_client_connect(SearpcNamedPipeClient *client);

LIBSEARPC_API
void searpc_free_client_with_pipe_transport (SearpcClient *client);

G_END_DECLS

#endif // SEARPC_NAMED_PIPE_TRANSPORT_H
