#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include <sys/stat.h>

#include <glib-object.h>

#include "searpc-client.h"
#include "searpc-demo-packet.h"

#define BUFLEN 256
#define MAGIC_STRING "ABCD"

#ifdef WIN32
    #include <inttypes.h>
    #include <winsock2.h>
    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif


static char *transport_callback(void *arg, const char *fcall_str,
                                size_t fcall_len, size_t *ret_len)
{
    int fd, ret;
    char buf[BUFLEN];
    packet *pac, *pac_ret;
   
    fd = (int)(long) arg;
    pac = (packet *)buf;

    /* construct the packet */
    pac->length = htons((uint16_t)fcall_len);
    memcpy(pac->data, fcall_str, fcall_len);

    /* send the packet */
    if ( writen (fd, buf, PACKET_HEADER_LENGTH + fcall_len) == -1) {
        fprintf (stderr, "write failed: %s\n", strerror(errno));
        exit(-1);
    }
    
    /* read the returned packet */
    pac_ret = read_packet(fd, buf);
    if (pac_ret == NULL) {
        fprintf(stderr, "read packet failed: %s\n", strerror(errno));
        exit(-1);
    }
    
    *ret_len = ntohs(pac_ret->length);

    return g_strndup(pac_ret->data, *ret_len);
}

int
main(int argc, char *argv[])
{
    int sockfd, ret;
    char *ret_str;
    struct sockaddr_in servaddr;
    SearpcClient *rpc_client;
    GError *error = NULL;

    g_type_init();

#ifdef WIN32
    WSADATA     wsadata;
    WSAStartup(0x0101, &wsadata);
#endif

    ret = sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0) {
        fprintf(stderr, "socket failed: %s\n", strerror(errno));
        exit(-1);
    }

    int on = 1;
    if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
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

    /* create an rpc_client and supply the transport function. */
    rpc_client = searpc_client_new();
    rpc_client->send = transport_callback;
    rpc_client->arg = (void *)(long)sockfd;

    /* call the client-side funcion */
    ret = searpc_client_call__int(rpc_client, "searpc_strlen", &error,
                                  1, "string", "hello searpc");
    if (error != NULL) {
        fprintf(stderr, "error: %s\n", error->message);
        exit(-1);
    } else
        printf("the length of string 'hello searpc' is %d.\n", ret);

    close(sockfd);

    return 0;
}
