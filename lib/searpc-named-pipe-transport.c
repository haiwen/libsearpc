#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <glib/gstdio.h>

#include "searpc-utils.h"
#include "searpc-client.h"
#include "searpc-server.h"
#include "searpc-named-pipe-transport.h"

static void* named_pipe_listen(void *arg);
static void* named_pipe_client_handler(void *arg);
static char* searpc_named_pipe_send(void *arg, const gchar *fcall_str, size_t fcall_len, size_t *ret_len);

static char * request_to_json(const char *service, const char *fcall_str, size_t fcall_len);

typedef struct {
    SearpcNamedPipeClient* client;
    char *service;
} ClientTransportData;

SearpcClient*
searpc_client_with_named_pipe_transport(SearpcNamedPipeClient *pipe_client,
                                        const char *service)
{
    SearpcClient *client= searpc_client_new();
    client->send = searpc_named_pipe_send;

    ClientTransportData *data = g_malloc(sizeof(ClientTransportData));
    data->client = pipe_client;
    data->service = g_strdup(service);

    client->arg = data;
    return client;
}

SearpcNamedPipeClient* searpc_create_named_pipe_client(const char *path)
{
    SearpcNamedPipeClient *client = g_malloc0(sizeof(SearpcNamedPipeClient));
    memcpy(client->path, path, strlen(path) + 1);
    return client;
}

SearpcNamedPipeServer* searpc_create_named_pipe_server(const char *path)
{
    SearpcNamedPipeServer *server = g_malloc0(sizeof(SearpcNamedPipeServer));
    memcpy(server->path, path, strlen(path) + 1);
    return server;
}

int searpc_named_pipe_server_start(SearpcNamedPipeServer *server)
{
    int pipe_fd = socket (AF_UNIX, SOCK_STREAM, 0);
    const char *un_path = server->path;
    if (pipe_fd < 0) {
        g_warning ("Failed to create unix socket fd : %s\n",
                   strerror(errno));
        return -1;
    }

    struct sockaddr_un saddr;
    saddr.sun_family = AF_UNIX;

    if (strlen(server->path) > sizeof(saddr.sun_path)-1) {
        g_warning ("Unix socket path %s is too long."
                       "Please set or modify UNIX_SOCKET option in ccnet.conf.\n",
                       un_path);
        goto failed;
    }

    if (g_file_test (un_path, G_FILE_TEST_EXISTS)) {
        g_warning ("socket file exists, delete it anyway\n");
        if (g_unlink (un_path) < 0) {
            g_warning ("delete socket file failed : %s\n", strerror(errno));
            goto failed;
        }
    }

    g_strlcpy (saddr.sun_path, un_path, sizeof(saddr.sun_path));
    if (bind(pipe_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        g_warning ("failed to bind unix socket fd to %s : %s\n",
                      un_path, strerror(errno));
        goto failed;
    }

    if (listen(pipe_fd, 3) < 0) {
        g_warning ("failed to listen to unix socket: %s\n", strerror(errno));
        goto failed;
    }

    if (chmod(un_path, 0700) < 0) {
        g_warning ("failed to set permisson for unix socket %s: %s\n",
                      un_path, strerror(errno));
        goto failed;
    }

    server->pipe_fd = pipe_fd;

    /* TODO: use glib thread pool */
    pthread_create(&server->listener_thread, NULL, named_pipe_listen, server);

    return 0;

failed:
    close(pipe_fd);

    return -1;
}

typedef struct {
    SearpcNamedPipeServer *server;
    int connfd;
} ServerHandlerData;

static void* named_pipe_listen(void *arg)
{
    SearpcNamedPipeServer *server = arg;
    while (1) {
        int connfd = accept (server->pipe_fd, NULL, 0);
        pthread_t *handler = g_malloc(sizeof(pthread_t));
        ServerHandlerData *data = g_malloc(sizeof(ServerHandlerData));
        data->server = server;
        data->connfd = connfd;
        // TODO(low priority): Instead of using a thread to handle each client,
        // use select(unix)/iocp(windows) to do it.
        pthread_create(handler, NULL, named_pipe_client_handler, data);
        server->handlers = g_list_append(server->handlers, handler);
    }
    return NULL;
}

static void* named_pipe_client_handler(void *arg)
{
    ServerHandlerData *data = arg;
    // SearpcNamedPipeServer *server = data->server;
    int connfd = data->connfd;

    size_t len;
    size_t bufsize = 4096;
    char *buf = g_malloc(bufsize);

    g_warning ("start to serve on pipe client\n");

    while (1) {
        if (pipe_read_n(connfd, &len, sizeof(uint32_t)) < 0) {
            g_warning("failed to read rpc request size: %s", strerror(errno));
            break;
        }

        while (bufsize < len) {
            bufsize *= 2;
            buf = realloc(buf, bufsize);
        }

        if (pipe_read_n(connfd, buf, len) < 0) {
            g_warning("failed to read rpc request: %s", strerror(errno));
            g_free (buf);
            break;
        }

        size_t ret_len;
        char *ret_str = searpc_server_call_function ("test", buf, len, &ret_len);

        if (pipe_write_n(connfd, &ret_len, sizeof(uint32_t)) < 0) {
            g_warning("failed to send rpc resopnse: %s", strerror(errno));
            g_free (ret_str);
            break;
        }

        if (pipe_write_n(connfd, ret_str, ret_len) < 0) {
            g_warning("failed to send rpc resopnse: %s", strerror(errno));
            g_free (ret_str);
            break;
        }
    }

    return NULL;
}


int searpc_named_pipe_client_connect(SearpcNamedPipeClient *client)
{
    client->pipe_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un servaddr;
    servaddr.sun_family = AF_UNIX;

    g_strlcpy (servaddr.sun_path, client->path, sizeof(servaddr.sun_path));
    if (connect(client->pipe_fd, (struct sockaddr *)&servaddr, (socklen_t)sizeof(servaddr)) < 0) {
        g_warning ("pipe client failed to connect to server\n");
        return -1;
    }
    g_warning ("pipe client connectd to server\n");
    return 0;
}

char *searpc_named_pipe_send(void *arg, const gchar *fcall_str,
                             size_t fcall_len, size_t *ret_len)
{
    g_warning ("searpc_named_pipe_send is called\n");
    ClientTransportData *data = arg;
    SearpcNamedPipeClient *client = data->client;

    char *json_str = request_to_json(data->service, fcall_str, fcall_len);
    size_t json_len = strlen(json_str);

    uint32_t len = fcall_len;
    if (pipe_write_n(client->pipe_fd, &len, sizeof(uint32_t)) < 0) {
        g_warning("failed to send rpc call: %s", strerror(errno));
        return NULL;
    }

    if (pipe_write_n(client->pipe_fd, json_str, json_len) < 0) {
        g_warning("failed to send rpc call: %s", strerror(errno));
        return NULL;
    }

    if (pipe_read_n(client->pipe_fd, &len, sizeof(uint32_t)) < 0) {
        g_warning("failed to read rpc response: %s", strerror(errno));
        return NULL;
    }

    char *buf = g_malloc(len);

    if (pipe_read_n(client->pipe_fd, buf, len) < 0) {
        g_warning("failed to read rpc response: %s", strerror(errno));
        g_free (buf);
        return NULL;
    }

    *ret_len = len;
    return buf;
}

static char *
request_to_json (const char *service, const char *fcall_str, size_t fcall_len)
{
    // TODO
    char *ret = g_malloc0(fcall_len + 1);
    memcpy(ret, fcall_str, fcall_len);
    return ret;

    /* json_t *object; */

    /* object = json_object (); */

    /* json_object_set_string_member (object, "commit_id", commit->commit_id); */
    /* json_object_set_string_member (object, "root_id", commit->root_id); */
    /* json_object_set_string_member (object, "repo_id", commit->repo_id); */
    /* if (commit->creator_name) */
    /*     json_object_set_string_member (object, "creator_name", commit->creator_name); */
    /* json_object_set_string_member (object, "creator", commit->creator_id); */
    /* json_object_set_string_member (object, "description", commit->desc); */
    /* json_object_set_int_member (object, "ctime", (gint64)commit->ctime); */
    /* json_object_set_string_or_null_member (object, "parent_id", commit->parent_id); */
    /* json_object_set_string_or_null_member (object, "second_parent_id", */
    /*                                        commit->second_parent_id); */
    /* /\* */
    /*  * also save repo's properties to commit file, for easy sharing of */
    /*  * repo info  */
    /*  *\/ */
    /* json_object_set_string_member (object, "repo_name", commit->repo_name); */
    /* json_object_set_string_member (object, "repo_desc", */
    /*                                commit->repo_desc); */
    /* json_object_set_string_or_null_member (object, "repo_category", */
    /*                                        commit->repo_category); */
    /* if (commit->device_name) */
    /*     json_object_set_string_member (object, "device_name", commit->device_name); */

    /* if (commit->encrypted) */
    /*     json_object_set_string_member (object, "encrypted", "true"); */

    /* if (commit->encrypted) { */
    /*     json_object_set_int_member (object, "enc_version", commit->enc_version); */
    /*     if (commit->enc_version >= 1) */
    /*         json_object_set_string_member (object, "magic", commit->magic); */
    /*     if (commit->enc_version == 2) */
    /*         json_object_set_string_member (object, "key", commit->random_key); */
    /* } */
    /* if (commit->no_local_history) */
    /*     json_object_set_int_member (object, "no_local_history", 1); */
    /* if (commit->version != 0) */
    /*     json_object_set_int_member (object, "version", commit->version); */
    /* if (commit->conflict) */
    /*     json_object_set_int_member (object, "conflict", 1); */
    /* if (commit->new_merge) */
    /*     json_object_set_int_member (object, "new_merge", 1); */
    /* if (commit->repaired) */
    /*     json_object_set_int_member (object, "repaired", 1); */

    /* return object; */
}
