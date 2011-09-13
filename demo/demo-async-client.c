#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glib-object.h>

#include "searpc-client.h"
#include "searpc-demo-packet.h"

#define BUFLEN 256
#define MAGIC_STRING "ABCD"

/* define the client-side function */
SEARPC_CLIENT_ASYNC_DEFUN_INT__STRING(searpc_strlen, 0);


typedef struct {
    int fd;
    void *rpc_priv;
} TcpTransport;

/* rpc_priv is used by the rpc_client to store information related to
 * this rpc call. */
static int transport_send(void *arg, const char *fcall_str,
                          size_t fcall_len, void *rpc_priv)
{
    TcpTransport *trans = arg;
    int fd, ret;
    char buf[BUFLEN];
    packet *pac, *pac_ret;
   
    pac = (packet *)buf;

    /* construct the packet */
    pac->length = htons((uint16_t)fcall_len);
    strncpy(pac->data, fcall_str, fcall_len);

    /* send the packet */
    if ( writen (trans->fd, buf, PACKET_HEADER_LENGTH + fcall_len) == -1) {
        fprintf (stderr, "write failed: %s\n", strerror(errno));
        exit(-1);
    }

    trans->rpc_priv = rpc_priv;
    
    return 0;
}

static void
transport_read(TcpTransport *trans)
{
    packet *pac;
    int ret_len;
    char buf[BUFLEN];

    /* read the returned packet */
    pac = read_packet(trans->fd, buf);
    if (pac == NULL) {
        fprintf(stderr, "read packet failed: %s\n", strerror(errno));
        exit(-1);
    }
    
    ret_len = ntohs(pac->length);
    searpc_client_generic_callback (pac->data, ret_len, trans->rpc_priv, NULL);
    trans->rpc_priv = NULL;
}

static void
strlen_callback(void *vresult, void *user_data, GError *error)
{
    const char *str = user_data;
    int len = (int)(long)vresult;
    
    g_assert (strcmp(str, "user data") == 0);
    printf("the length of string 'hello searpc' is %d.\n", len);
}

int
main(int argc, char *argv[])
{
    int sockfd, ret;
    char *ret_str;
    struct sockaddr_in servaddr;
    SearpcClient *rpc_client;
    GError *error = NULL;
    TcpTransport *transport;

    g_type_init();

    ret = sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0) {
        fprintf(stderr, "socket failed: %s\n", strerror(errno));
        exit(-1);
    }

    int on = 1;
    if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        fprintf (stderr, "setsockopt of SO_REUSEADDR error: %s\n", strerror(errno));
        exit(-1);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret < 0) {
        fprintf(stderr, "connect failed: %s\n", strerror(errno));
        exit(-1);
    }

    transport = g_new0 (TcpTransport, 1);
    transport->fd = sockfd;

    /* create an rpc_client and supply the transport function. */
    rpc_client = searpc_client_new();
    rpc_client->async_send = transport_send;
    rpc_client->async_arg = (void *)(long)transport;

    /* call the client-side funcion */
    searpc_strlen(rpc_client, "hello searpc", strlen_callback, "user data");

    /* call the transport to receive response */
    transport_read (transport);
    if (error != NULL) {
        fprintf(stderr, "error: %s\n", error->message);
        exit(-1);
    }

    close(sockfd);

    return 0;
}
