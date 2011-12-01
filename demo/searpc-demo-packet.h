
#ifndef _PACKET_H
#define _PACKET_H

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#ifdef WIN32
    #include <windows.h>
    #include <inttypes.h>
    #include <winsock2.h>
    #include <ctype.h>
    #include <ws2tcpip.h>
    #define UNUSED 
#endif

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
#ifdef WIN32
        if ( (nwritten = send(fd, ptr, nleft,0)) <= 0) {
#else
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
#endif
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
#ifdef WIN32
        if ( (nread = recv(fd, buf, nleft, 0)) < 0) {
#else
		if ( (nread = read(fd, buf, nleft)) < 0) {
#endif          
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


#ifdef WIN32


#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#endif

#ifndef IN6ADDRSZ
#define IN6ADDRSZ       16
#endif

#ifndef INT16SZ
#define INT16SZ         2
#endif

#ifndef INADDRSZ
#define INADDRSZ        4
#endif

/*
 *  Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

/* int
 * inet_pton4(src, dst, pton)
 *      when last arg is 0: inet_aton(). with hexadecimal, octal and shorthand.
 *      when last arg is 1: inet_pton(). decimal dotted-quad only.
 * return:
 *      1 if `src' is a valid input, else 0.
 * notice:
 *      does not touch `dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
int inet_pton4(const char *src, u_char *dst, int pton)
{
    u_int val;
    u_int digit;
    int base, n;
    unsigned char c;
    u_int parts[4];
    register u_int *pp = parts;

    c = *src;
    for (;;) {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, isdigit=decimal.
         */
        if (!isdigit(c))
            return (0);
        val = 0; base = 10;
        if (c == '0') {
            c = *++src;
            if (c == 'x' || c == 'X')
                base = 16, c = *++src;
            else if (isdigit(c) && c != '9')
                base = 8;
        }
        /* inet_pton() takes decimal only */
        if (pton && base != 10)
            return (0);
        for (;;) {
            if (isdigit(c)) {
                digit = c - '0';
                if (digit >= base)
                    break;
                val = (val * base) + digit;
                c = *++src;
            } else if (base == 16 && isxdigit(c)) {
                digit = c + 10 - (islower(c) ? 'a' : 'A');
                if (digit >= 16)
                    break;
                val = (val << 4) | digit;
                c = *++src;
            } else
                break;
        }
        if (c == '.') {
            /*
             * Internet format:
             *      a.b.c.d
             *      a.b.c   (with c treated as 16 bits)
             *      a.b     (with b treated as 24 bits)
             *      a       (with a treated as 32 bits)
             */
            if (pp >= parts + 3)
                return (0);
            *pp++ = val;
            c = *++src;
        } else
            break;
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !isspace(c))
        return (0);
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    n = pp - parts + 1;
    /* inet_pton() takes dotted-quad only.  it does not take shorthand. */
    if (pton && n != 4)
        return (0);
    switch (n) {

    case 0:
        return (0);             /* initial nondigit */

    case 1:                         /* a -- 32 bits */
        break;

    case 2:                         /* a.b -- 8.24 bits */
        if (parts[0] > 0xff || val > 0xffffff)
            return (0);
        val |= parts[0] << 24;
        break;

    case 3:                         /* a.b.c -- 8.8.16 bits */
        if ((parts[0] | parts[1]) > 0xff || val > 0xffff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4:                         /* a.b.c.d -- 8.8.8.8 bits */
        if ((parts[0] | parts[1] | parts[2] | val) > 0xff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    }
    if (dst) {
        val = htonl(val);
        memcpy(dst, &val, INADDRSZ);
    }
    return (1);
}

/* int
 * inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
int inet_pton6(const char *src, u_char *dst)
{
    static const char xdigits_l[] = "0123456789abcdef",
        xdigits_u[] = "0123456789ABCDEF";
    u_char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
    const char *xdigits, *curtok;
    int ch, saw_xdigit;
    u_int val;

    memset((tp = tmp), '\0', IN6ADDRSZ);
    endp = tp + IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':')
            return (0);
    curtok = src;
    saw_xdigit = 0;
    val = 0;
    while ((ch = *src++) != '\0') {
        const char *pch;

        if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
            pch = strchr((xdigits = xdigits_u), ch);
        if (pch != NULL) {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return (0);
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':') {
            curtok = src;
            if (!saw_xdigit) {
                if (colonp)
                    return (0);
                colonp = tp;
                continue;
            } else if (*src == '\0')
                return (0);
            if (tp + INT16SZ > endp)
                return (0);
            *tp++ = (u_char) (val >> 8) & 0xff;
            *tp++ = (u_char) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
            inet_pton4(curtok, tp, 1) > 0) {
            tp += INADDRSZ;
            saw_xdigit = 0;
            break;  /* '\0' was seen by inet_pton4(). */
        }
        return (0);
    }
    if (saw_xdigit) {
        if (tp + INT16SZ > endp)
            return (0);
        *tp++ = (u_char) (val >> 8) & 0xff;
        *tp++ = (u_char) val & 0xff;
    }
    if (colonp != NULL) {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        if (tp == endp)
            return (0);
        for (i = 1; i <= n; i++) {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return (0);
    memcpy(dst, tmp, IN6ADDRSZ);
    return (1);
}

/* int
 * inet_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *      Paul Vixie, 1996.
 */
int inet_pton(int af, const char *src, void *dst)
{
     switch (af) {
    case AF_INET:
        return (inet_pton4(src, dst, 1));
    case AF_INET6:
        return (inet_pton6(src, dst));
    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
    /* NOTREACHED */
}
#endif  /* WIN32 */
