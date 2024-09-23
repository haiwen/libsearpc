#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#if !defined(WIN32)
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <unistd.h>
#ifdef __linux__
  #include <fcntl.h>
  #include <sys/epoll.h>
#endif
#endif // !defined(WIN32)

#include <glib.h>
#include <glib/gstdio.h>
#include <jansson.h>

#include "searpc-utils.h"
#include "searpc-client.h"
#include "searpc-server.h"
#include "searpc-named-pipe-transport.h"

#if defined(WIN32)
static const int kPipeBufSize = 1024;
static char* formatErrorMessage();

#define G_WARNING_WITH_LAST_ERROR(fmt)                        \
    do {                                                      \
        char *error_msg__ = formatErrorMessage();             \
        g_warning(fmt ": %s\n", error_msg__);                 \
        g_free (error_msg__);                                 \
    } while(0);

#endif // defined(WIN32)

static void* named_pipe_listen(void *arg);
static void* handle_named_pipe_client_with_thread (void *arg);
static void handle_named_pipe_client_with_threadpool(void *data, void *user_data);
static void named_pipe_client_handler (void *data);
static char* searpc_named_pipe_send(void *arg, const gchar *fcall_str, size_t fcall_len, size_t *ret_len);

static char * request_to_json(const char *service, const char *fcall_str, size_t fcall_len);
static int request_from_json (const char *content, size_t len, char **service, char **fcall_str);
static void json_object_set_string_member (json_t *object, const char *key, const char *value);
static const char * json_object_get_string_member (json_t *object, const char *key);

static gssize pipe_write_n(SearpcNamedPipe fd, const void *vptr, size_t n);
static gssize pipe_read_n(SearpcNamedPipe fd, void *vptr, size_t n);

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

SearpcNamedPipeServer* searpc_create_named_pipe_server_with_threadpool (const char *path, int named_pipe_server_thread_pool_size)
{
    GError *error = NULL;

    SearpcNamedPipeServer *server = g_malloc0(sizeof(SearpcNamedPipeServer));
    memcpy(server->path, path, strlen(path) + 1);
    server->pool_size = named_pipe_server_thread_pool_size;
    server->named_pipe_server_thread_pool = g_thread_pool_new (handle_named_pipe_client_with_threadpool,
                                                               NULL,
                                                               named_pipe_server_thread_pool_size,
                                                               FALSE,
                                                               &error);
    if (!server->named_pipe_server_thread_pool) {
        if (error) {
            g_warning ("Falied to create named pipe server thread pool : %s\n", error->message);
            g_clear_error (&error);
        } else {
            g_warning ("Falied to create named pipe server thread pool.\n");
        }
        g_free (server);
        return NULL;
    }

    return server;
}

int searpc_named_pipe_server_start(SearpcNamedPipeServer *server)
{
#if !defined(WIN32)
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
        g_message ("socket file exists, delete it anyway\n");
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

    if (listen(pipe_fd, 10) < 0) {
        g_warning ("failed to listen to unix socket: %s\n", strerror(errno));
        goto failed;
    }

    if (chmod(un_path, 0700) < 0) {
        g_warning ("failed to set permisson for unix socket %s: %s\n",
                      un_path, strerror(errno));
        goto failed;
    }

#ifdef __linux__
    if (server->use_epoll) {
        int epoll_fd;
        struct epoll_event event;

        epoll_fd = epoll_create1(0);
        if (epoll_fd < 0) {
            g_warning ("failed to open an epoll file descriptor: %s\n", strerror(errno));
            goto failed;
        }

        event.events = EPOLLIN;
        event.data.fd = pipe_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fd, &event) == -1) {
            g_warning ("failed to add pipe fd to epoll list: %s\n", strerror(errno));
            goto failed;
        }

        server->epoll_fd = epoll_fd;
    }
#endif

    server->pipe_fd = pipe_fd;

#endif // !defined(WIN32)

    /* TODO: use glib thread pool */
    pthread_create(&server->listener_thread, NULL, named_pipe_listen, server);
    return 0;

#if !defined(WIN32)
failed:
    close(pipe_fd);
    return -1;
#endif
}

typedef struct {
    SearpcNamedPipe connfd;
    char *buffer;
    guint32 len;
    guint32 header_offset;
    guint32 offset;
    SearpcNamedPipeServer *server;
    gboolean use_epoll;
} ServerHandlerData;

// EPOLL
#ifdef __linux__

// This function will keep reading until the following conditions are met:
// 1. the socket has beed closed.
// 2. there is no more readable data in the system cache;
// 3. the requested size has been read;
// 4. an unrecoverable error has been encountered;
// The first two cases are not errors.
gssize
epoll_read_n(int fd, void *vptr, size_t n)
{
    size_t  nleft;
    gssize nread;
    char    *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;      /* and call read() again */
            else if (errno == EAGAIN)
                break;
            else
                return(-1);
        } else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);      /* return >= 0 */
}

// This function will keep writing until the following conditions are met:
// 1. the socket has beed closed.
// 2. the requested size has been writed;
// 3. an unrecoverable error has been encountered;
// The first two cases are not errors.
gssize
epoll_write_n(int fd, const void *vptr, size_t n)
{
    size_t      nleft;
    gssize     nwritten;
    const char  *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (errno == EAGAIN)
                nwritten = 0;       /* and call write() again */
            else if (nwritten < 0 && errno == EINTR)
                nwritten = 0;       /* and call write() again */
            else
                return(-1);         /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

static void epoll_handler(void *data)
{
    ServerHandlerData *handler_data = data;
    SearpcNamedPipe connfd = handler_data->connfd;
    SearpcNamedPipeServer *server = handler_data->server;
    const char *buf = handler_data->buffer;
    guint32 len = handler_data->len;
    char *ret_str = NULL;
    int ret = 0;

    char *service, *body;
    if (request_from_json (buf, len, &service, &body) < 0) {
        ret = -1;
        goto out;
    }

    gsize ret_len;
    ret_str = searpc_server_call_function (service, body, strlen(body), &ret_len);
    g_free (service);
    g_free (body);

    len = (guint32)ret_len;
    if (epoll_write_n(connfd, &len, sizeof(guint32)) < 0) {
        ret = -1;
        goto out;
    }

    if (epoll_write_n(connfd, ret_str, ret_len) < 0) {
        ret = -1;
        goto out;
    }

    g_free (handler_data->buffer);
    handler_data->buffer = NULL;
    handler_data->len = 0;
    handler_data->header_offset = 0;
    handler_data->offset = 0;
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.ptr = (void *)handler_data;

    if (epoll_ctl (server->epoll_fd, EPOLL_CTL_ADD, connfd, &event) == -1) {
        ret = -1;
        g_warning ("failed to add client fd to epoll list: %s\n", strerror(errno));
        goto out;
    }

out:
    if (ret < 0) {
        close (connfd);
        g_free (handler_data->buffer);
        g_free (handler_data);
    }
    g_free (ret_str);
}

static int
read_client_request (int connfd, ServerHandlerData *data)
{
    char *buf;
    int n;

    // read length of request.
    if (data->header_offset != 4) {
        n = epoll_read_n(connfd, (void *)&(data->len) + data->header_offset, sizeof(guint32) - data->header_offset);
        if (n < 0) {
            g_warning("Failed to read rpc request size: %s\n", strerror(errno));
            return -1;
        }
        data->header_offset += n;
        if (data->header_offset < 4) {
            return 0;
        } else if (data->header_offset > 4) {
            g_warning("Failed to read rpc request size\n");
            return -1;
        }
        if (data->len == 0) {
            return -1;
        }
    }

    if (!data->buffer) {
        data->buffer = g_new0 (char, data->len);
    }

    // read content of request.
    n = epoll_read_n(connfd, data->buffer + data->offset, data->len - data->offset);
    if (n < 0) {
        g_warning ("Failed to read rpc request: %s\n", strerror(errno));
        return -1;
    }
    data->offset += n;

    return 0;
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void
epoll_listen (SearpcNamedPipeServer *server)
{
#define MAX_EVENTS 1000
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    int connfd;
    int n_events;
    int i;

    while (1) {
        n_events = epoll_wait (server->epoll_fd, events, MAX_EVENTS, 1000);
        if (n_events <= 0) {
            if (n_events < 0) {
                g_warning ("Failed to call waits for events on the epoll: %s\n", strerror(errno));
            }
            continue;
        }
        for (i = 0; i < n_events; i++) {
            if (events[i].data.fd == server->pipe_fd) {
                if (!(events[i].events & EPOLLIN)) {
                    continue;
                }
                // new client connection
                connfd = accept (server->pipe_fd, NULL, 0);
                if (connfd < 0) {
                    g_warning ("Failed to accept new client connection: %s\n", strerror(errno));
                    continue;
                }

                if (set_nonblocking(connfd) < 0) {
                    close(connfd);
                    g_warning ("Failed to set connection to noblocking.\n");
                    continue;
                }

                event.events = EPOLLIN | EPOLLRDHUP;
                ServerHandlerData *data = g_new0(ServerHandlerData, 1);
                data->use_epoll = TRUE;
                data->connfd = connfd;
                data->server = server;
                event.data.ptr = (void *)data;
                if (epoll_ctl (server->epoll_fd, EPOLL_CTL_ADD, connfd, &event) == -1) {
                    g_warning ("Failed to add client fd to epoll list: %s\n", strerror(errno));
                    epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, connfd, NULL);
                    close (connfd);
                    g_free (data);
                    continue;
                }
                g_message ("start to serve on pipe client\n");
            } else {
                ServerHandlerData *data = (ServerHandlerData *)events[i].data.ptr;
                connfd = data->connfd;
                if (events[i].events & (EPOLLHUP | EPOLLRDHUP)) {
                    if (data->len > 0)
                        g_free (data->buffer);
                    g_free (data);
                    epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, connfd, NULL);
                    close (connfd);
                    continue;
                }
                int rc = read_client_request (connfd, data);
                if (rc < 0) {
                    if (data->len)
                        g_free (data->buffer);
                    g_free (data);
                    epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, connfd, NULL);
                    close (connfd);
                    continue;
                } else if (data->header_offset != 4 || data->len != data->offset) {
                    // Continue reading request.
                    continue;
                }

                // After reading the contents of a request, remove the socket from the epoll and do not read the next request for a while to prevent client errors.
                // After the worker finishes processing the current request, we will add the socket back into the epoll.
                epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, connfd, NULL);
                if (server->named_pipe_server_thread_pool) {
                    if (g_thread_pool_get_num_threads (server->named_pipe_server_thread_pool) >= server->pool_size) {
                        g_warning("The rpc server thread pool is full, the maximum number of threads is %d\n", server->pool_size);
                    }
                    g_thread_pool_push (server->named_pipe_server_thread_pool, data, NULL);
                } else {
                    pthread_t handler;
                    pthread_attr_t attr;
                    pthread_attr_init(&attr);
                    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                    pthread_create(&handler, &attr, handle_named_pipe_client_with_thread, data);
                }
            }
        }
    }
}
#endif

static void* named_pipe_listen(void *arg)
{
    SearpcNamedPipeServer *server = arg;

#if !defined(WIN32)
#ifdef __linux__
    if (server->use_epoll) {
        epoll_listen (server);
        return NULL;
    }
#endif
    while (1) {
        int connfd = accept (server->pipe_fd, NULL, 0);
        ServerHandlerData *data = g_malloc(sizeof(ServerHandlerData));
        data->connfd = connfd;
        data->use_epoll = FALSE;
        if (server->named_pipe_server_thread_pool) {
            if (g_thread_pool_get_num_threads (server->named_pipe_server_thread_pool) >= server->pool_size) {
                g_warning("The rpc server thread pool is full, the maximum number of threads is %d\n", server->pool_size);
            }
            g_thread_pool_push (server->named_pipe_server_thread_pool, data, NULL);
        } else {
            pthread_t handler;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&handler, &attr, handle_named_pipe_client_with_thread, data);
        }
    }
#else // !defined(WIN32)
    while (1) {
        HANDLE connfd = INVALID_HANDLE_VALUE;
        BOOL connected = FALSE;

        connfd = CreateNamedPipe(
            server->path,             // pipe name
            PIPE_ACCESS_DUPLEX,       // read/write access
            PIPE_TYPE_MESSAGE |       // message type pipe
            PIPE_READMODE_MESSAGE |   // message-read mode
            PIPE_WAIT,                // blocking mode
            PIPE_UNLIMITED_INSTANCES, // max. instances
            kPipeBufSize,             // output buffer size
            kPipeBufSize,             // input buffer size
            0,                        // client time-out
            NULL);                    // default security attribute

        if (connfd == INVALID_HANDLE_VALUE) {
            G_WARNING_WITH_LAST_ERROR ("Failed to create named pipe");
            break;
        }

        /* listening on this pipe */
        connected = ConnectNamedPipe(connfd, NULL) ?
            TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (!connected) {
            G_WARNING_WITH_LAST_ERROR ("failed to ConnectNamedPipe()");
            CloseHandle(connfd);
            break;
        }

        /* g_debug ("Accepted a named pipe client\n"); */

        ServerHandlerData *data = g_malloc(sizeof(ServerHandlerData));
        data->connfd = connfd;
        data->use_epoll = FALSE;
        if (server->named_pipe_server_thread_pool)
            g_thread_pool_push (server->named_pipe_server_thread_pool, data, NULL);
        else {
            pthread_t handler;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&handler, &attr, handle_named_pipe_client_with_thread, data);
        }
    }
#endif // !defined(WIN32)
    return NULL;
}

static void* handle_named_pipe_client_with_thread(void *arg)
{
#ifdef __linux__
    ServerHandlerData *handler_data = arg;
    if (handler_data->use_epoll) {
        epoll_handler (arg);
        return NULL;
    }
#endif
    named_pipe_client_handler(arg);

    return NULL;
}

static void handle_named_pipe_client_with_threadpool(void *data, void *user_data)
{
#ifdef __linux__
    ServerHandlerData *handler_data = data;
    if (handler_data->use_epoll) {
        epoll_handler (data);
        return;
    }
#endif
    named_pipe_client_handler(data);
}

static void named_pipe_client_handler(void *data)
{
    ServerHandlerData *handler_data = data;
    SearpcNamedPipe connfd = handler_data->connfd;

    guint32 len;
    guint32 bufsize = 4096;
    char *buf = g_malloc(bufsize);

    g_message ("start to serve on pipe client\n");

    while (1) {
        len = 0;
        if (pipe_read_n(connfd, &len, sizeof(guint32)) < 0) {
            g_warning("failed to read rpc request size: %s\n", strerror(errno));
            break;
        }

        if (len == 0) {
            /* g_debug("EOF reached, pipe connection lost"); */
            break;
        }

        while (bufsize < len) {
            bufsize *= 2;
            buf = realloc(buf, bufsize);
        }

        if (pipe_read_n(connfd, buf, len) < 0 || len == 0) {
            g_warning("failed to read rpc request: %s\n", strerror(errno));
            break;
        }

        char *service, *body;
        if (request_from_json (buf, len, &service, &body) < 0) {
            break;
        }

        gsize ret_len;
        char *ret_str = searpc_server_call_function (service, body, strlen(body), &ret_len);
        g_free (service);
        g_free (body);

        len = (guint32)ret_len;
        if (pipe_write_n(connfd, &len, sizeof(guint32)) < 0) {
            g_warning("failed to send rpc response(%s): %s\n", ret_str, strerror(errno));
            g_free (ret_str);
            break;
        }

        if (pipe_write_n(connfd, ret_str, ret_len) < 0) {
            g_warning("failed to send rpc response: %s\n", strerror(errno));
            g_free (ret_str);
            break;
        }

        g_free (ret_str);
    }

#if !defined(WIN32)
    close(connfd);
#else // !defined(WIN32)
    DisconnectNamedPipe(connfd);
    CloseHandle(connfd);
#endif // !defined(WIN32)
    g_free (data);
    g_free (buf);
}

int searpc_named_pipe_client_connect(SearpcNamedPipeClient *client)
{
#if !defined(WIN32)
    client->pipe_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un servaddr;
    servaddr.sun_family = AF_UNIX;

    g_strlcpy (servaddr.sun_path, client->path, sizeof(servaddr.sun_path));
    if (connect(client->pipe_fd, (struct sockaddr *)&servaddr, (socklen_t)sizeof(servaddr)) < 0) {
        g_warning ("pipe client failed to connect to server: %s\n", strerror(errno));
        close(client->pipe_fd);
        return -1;
    }

#else // !defined(WIN32)
    SearpcNamedPipe pipe_fd;

    for (;;) {
        pipe_fd = CreateFile(
            client->path,           // pipe name
            GENERIC_READ |          // read and write access
            GENERIC_WRITE,
            0,                      // no sharing
            NULL,                   // default security attributes
            OPEN_EXISTING,          // opens existing pipe
            0,                      // default attributes
            NULL);                  // no template file

        if (pipe_fd != INVALID_HANDLE_VALUE) {
            break;
        }

        /* wait with default timeout (approx. 50ms) */
        if (GetLastError() != ERROR_PIPE_BUSY || !WaitNamedPipe(client->path, NMPWAIT_USE_DEFAULT_WAIT)) {
            G_WARNING_WITH_LAST_ERROR("Failed to connect to named pipe");
            return -1;
        }
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipe_fd, &mode, NULL, NULL)) {
        G_WARNING_WITH_LAST_ERROR("Failed to set named pipe mode");
        CloseHandle (pipe_fd);
        return -1;
    }

    client->pipe_fd = pipe_fd;

#endif // !defined(WIN32)

    /* g_debug ("pipe client connected to server\n"); */
    return 0;
}

void searpc_free_client_with_pipe_transport (SearpcClient *client)
{
    ClientTransportData *data = (ClientTransportData *)(client->arg);
    SearpcNamedPipeClient *pipe_client = data->client;
#if defined(WIN32)
    CloseHandle(pipe_client->pipe_fd);
#else
    close(pipe_client->pipe_fd);
#endif
    g_free (pipe_client);
    g_free (data->service);
    g_free (data);
    searpc_client_free (client);
}

char *searpc_named_pipe_send(void *arg, const gchar *fcall_str,
                             size_t fcall_len, size_t *ret_len)
{
    /* g_debug ("searpc_named_pipe_send is called\n"); */
    ClientTransportData *data = arg;
    SearpcNamedPipeClient *client = data->client;

    char *json_str = request_to_json(data->service, fcall_str, fcall_len);
    guint32 len = (guint32)strlen(json_str);

    if (pipe_write_n(client->pipe_fd, &len, sizeof(guint32)) < 0) {
        g_warning("failed to send rpc call: %s\n", strerror(errno));
        free (json_str);
        return NULL;
    }

    if (pipe_write_n(client->pipe_fd, json_str, len) < 0) {
        g_warning("failed to send rpc call: %s\n", strerror(errno));
        free (json_str);
        return NULL;
    }

    free (json_str);

    if (pipe_read_n(client->pipe_fd, &len, sizeof(guint32)) < 0) {
        g_warning("failed to read rpc response: %s\n", strerror(errno));
        return NULL;
    }

    char *buf = g_malloc(len);

    if (pipe_read_n(client->pipe_fd, buf, len) < 0) {
        g_warning("failed to read rpc response: %s\n", strerror(errno));
        g_free (buf);
        return NULL;
    }

    *ret_len = len;
    return buf;
}

static char *
request_to_json (const char *service, const char *fcall_str, size_t fcall_len)
{
    json_t *object = json_object ();

    char *temp_request = g_malloc0(fcall_len + 1);
    memcpy(temp_request, fcall_str, fcall_len);

    json_object_set_string_member (object, "service", service);
    json_object_set_string_member (object, "request", temp_request);

    g_free (temp_request);

    char *str = json_dumps (object, 0);
    json_decref (object);
    return str;
}

static int
request_from_json (const char *content, size_t len, char **service, char **fcall_str)
{
    json_error_t jerror;
    json_t *object = json_loadb(content, len, 0, &jerror);
    if (!object) {
        g_warning ("Failed to parse request body: %s.\n", strlen(jerror.text) > 0 ? jerror.text : "");
        return -1;
    }

    *service = g_strdup(json_object_get_string_member (object, "service"));
    *fcall_str = g_strdup(json_object_get_string_member(object, "request"));

    json_decref (object);

    if (!*service || !*fcall_str) {
        g_free (*service);
        g_free (*fcall_str);
        return -1;
    }

    return 0;
}

static void json_object_set_string_member (json_t *object, const char *key, const char *value)
{
    json_object_set_new (object, key, json_string (value));
}

static const char *
json_object_get_string_member (json_t *object, const char *key)
{
    json_t *string = json_object_get (object, key);
    if (!string)
        return NULL;
    return json_string_value (string);
}

#if !defined(WIN32)

// Write "n" bytes to a descriptor.
gssize
pipe_write_n(int fd, const void *vptr, size_t n)
{
    size_t      nleft;
    gssize     nwritten;
    const char  *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;       /* and call write() again */
            else
                return(-1);         /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

// Read "n" bytes from a descriptor.
gssize
pipe_read_n(int fd, void *vptr, size_t n)
{
    size_t  nleft;
    gssize nread;
    char    *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;      /* and call read() again */
            else
                return(-1);
        } else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);      /* return >= 0 */
}

#else // !defined(WIN32)

gssize pipe_read_n (SearpcNamedPipe fd, void *vptr, size_t n)
{
    DWORD bytes_read;
    BOOL success = ReadFile(
        fd,                     // handle to pipe
        vptr,                   // buffer to receive data
        (DWORD)n,               // size of buffer
        &bytes_read,            // number of bytes read
        NULL);                  // not overlapped I/O

    if (!success || bytes_read != (DWORD)n) {
        if (GetLastError() == ERROR_BROKEN_PIPE) {
            return 0;
        }
        G_WARNING_WITH_LAST_ERROR("failed to read from pipe");
        return -1;
    }

    return n;
}

gssize pipe_write_n(SearpcNamedPipe fd, const void *vptr, size_t n)
{
    DWORD bytes_written;
    BOOL success = WriteFile(
        fd,                     // handle to pipe
        vptr,                   // buffer to receive data
        (DWORD)n,               // size of buffer
        &bytes_written,         // number of bytes written
        NULL);                  // not overlapped I/O

    if (!success || bytes_written != (DWORD)n) {
        G_WARNING_WITH_LAST_ERROR("failed to write to named pipe");
        return -1;
    }

    FlushFileBuffers(fd);
    return 0;
}

// http://stackoverflow.com/questions/3006229/get-a-text-from-the-error-code-returns-from-the-getlasterror-function
// The caller is responsible to free the returned message.
char* formatErrorMessage()
{
    DWORD error_code = GetLastError();
    if (error_code == 0) {
        return g_strdup("no error");
    }
    char buf[256] = {0};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   error_code,
                   /* EN_US */
                   MAKELANGID(LANG_ENGLISH, 0x01),
                   buf,
                   sizeof(buf) - 1,
                   NULL);
    return g_strdup(buf);
}

#endif // !defined(WIN32)
