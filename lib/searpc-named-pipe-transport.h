#ifndef SEARPC_NAMED_PIPE_TRANSPORT_H
#define SEARPC_NAMED_PIPE_TRANSPORT_H

#include <glib.h>
#include <glib-object.h>
#include <jansson.h>

typedef struct {
    char path[4096];
    pthread_t listener_thread;
    GList *handlers;
    int pipe_fd;
} SearpcNamedPipeServer;

typedef struct {
    char path[4096];
    int pipe_fd;
} SearpcNamedPipeClient;

SearpcNamedPipeClient* searpc_create_named_pipe_client(const char *path);

SearpcNamedPipeServer* searpc_create_named_pipe_server(const char *path);

SearpcClient * searpc_client_with_named_pipe_transport(SearpcNamedPipeClient *client, const char *service);

int searpc_named_pipe_server_start(SearpcNamedPipeServer *server);

int searpc_named_pipe_client_connect(SearpcNamedPipeClient *client);

#endif // SEARPC_NAMED_PIPE_TRANSPORT_H
