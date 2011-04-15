
#ifndef _PACKET_H
#define _PACKET_H

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

typedef struct packet {
    uint16_t length;
    char data[0];
} packet;

# define PACKET_HEADER_LENGTH sizeof(packet)

static ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

static ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, char *buf, size_t n)
{
	size_t	nleft;
	ssize_t	nread;

	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, buf, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
        buf += nread;
	}

	return(n - nleft);		/* return >= 0 */    
}


/* read a packet into a buffer, and return the pacet pointer */
packet *
read_packet(int sockfd, char *buf)
{
    packet *pac;
    int len;
    /* read the length part */
    if (readn (sockfd, buf, PACKET_HEADER_LENGTH) != PACKET_HEADER_LENGTH) {
        fprintf(stderr, "read header error: %s\n", strerror(errno));
        exit(-1);
    }
    pac = (packet *)buf;
    len = ntohs(pac->length);
    /* read the data */
    if (len <= 0)
        return NULL;
    else if (readn (sockfd, buf + PACKET_HEADER_LENGTH, len) != len)
        return NULL;

    return pac;
}
            
#endif


