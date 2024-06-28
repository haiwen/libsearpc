#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include <sys/stat.h>

#include <glib-object.h>

#include <searpc.h>

#include "searpc-demo-packet.h"
#include "test-object.h"

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

/*The function is copied from searpc-server.c for convience*/
void
searpc_set_objlist_to_ret_object (json_t *object, GList *ret)
{
    GList *ptr;

    if (ret == NULL)
        json_object_set_new (object, "ret", json_null ());
    else {
        json_t *array = json_array ();
        for (ptr = ret; ptr; ptr = ptr->next)
            json_array_append_new (array, json_gobject_serialize (ptr->data));
        json_object_set_new (object, "ret", array);

        for (ptr = ret; ptr; ptr = ptr->next)
            g_object_unref (ptr->data);
        g_list_free (ret);
    }
}

int
connection_init(int *sockfd, struct sockaddr_in *servaddr)
{
    int ret;
    ret = *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0) {
        fprintf(stderr, "socket failed: %s\n", strerror(errno));
        return -1;
    }
    int on = 1;
    if (setsockopt (*sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
        fprintf (stderr, "setsockopt of SO_REUSEADDR error: %s\n", strerror(errno));
        return -1;
    }

    ret = connect(*sockfd, (struct sockaddr *)servaddr, sizeof(*servaddr));
    if (ret < 0) {
        fprintf(stderr, "connect failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

void
rpc_string_test(int sockfd, struct sockaddr_in *servaddr, SearpcClient *rpc_client, GError *error)
{
    int ret;
    char str[16] = "hello searpc";
    /* call the client-side funcion */
    ret = searpc_client_call__int(rpc_client, "searpc_strlen", &error,
                                  1, "string", str);
    if (error != NULL) {
        fprintf(stderr, "error: %s\n", error->message);
        exit(-1);
    } else
        printf("the length of string %s is %d.\n", str, ret);

    if (ret == strlen(str))
        printf("String test succeed.\n");
    else printf("String test fail.\n");
    close(sockfd);
}

void
rpc_glist_test(int sockfd, struct sockaddr_in *servaddr, SearpcClient *rpc_client, GError *error)
{
    int count = 4, len = 11;
    char str[16] = "A rpc test.";
    GList *ans=searpc_client_call__objlist(rpc_client, "searpc_objlisttest",
                                           TEST_OBJECT_TYPE, &error, 3,
                                           "int", count,
                                           "int", len,
                                           "string", str);

    json_t *object=json_object();
    searpc_set_objlist_to_ret_object (object,ans);

    if (error != NULL) {
        fprintf(stderr, "error: %s\n", error->message);
        exit(-1);
    }
    else printf("%s\n", json_dumps (object, JSON_INDENT(2)));

    json_t *array = json_object_get (object, "ret");
    if (json_array_size(array) != count) {
        printf("Glisttest fail.\n");
        return;
    }
    int i;
    for (i = 0; i != count; ++i) {
        json_t *member = json_array_get(array, i);
        if (json_integer_value (json_object_get (member, "len"))!=len) {
            printf("Glisttest fail.\n");
            return;
        }
        if (strcmp (json_string_value (json_object_get (member, "str")), str)) {
            printf("Glisttest fail.\n");
            return;
        }
        if ((json_is_false (json_object_get (member, "equal"))) == (strlen(str)==len)) {
            printf("Glisttest fail.\n");
            return;
        }
    }

    json_decref(object);
    printf("Glisttest succeed.\n");
    close(sockfd);
}

int
main(int argc, char *argv[])
{
    int sockfd;
    char *ret_str;
    struct sockaddr_in servaddr;
    SearpcClient *rpc_client;
    GError *error = NULL;

#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
#endif

#ifdef WIN32
    WSADATA     wsadata;
    WSAStartup(0x0101, &wsadata);
#endif

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if (connection_init(&sockfd, &servaddr)<0) exit(-1);

    /* create an rpc_client and supply the transport function. */
    rpc_client = searpc_client_new();
    rpc_client->send = transport_callback;
    rpc_client->arg = (void *)(long)sockfd;

    rpc_string_test(sockfd, &servaddr, rpc_client, error);

    if (connection_init(&sockfd, &servaddr)<0) exit(-1);
    rpc_client->arg = (void *)(long)sockfd;

    rpc_glist_test(sockfd, &servaddr, rpc_client, error);

    return 0;
}
