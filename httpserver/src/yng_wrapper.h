/*
 * Author: Jimmy Young
 * yng_wrapper.h
 */


#ifndef _YNG_WRAPPER_
#define _YNG_WRAPPER_


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#include "yng_log.h"


int yng_socket(int domain, int type, int protocol);
int yng_close(int sockfd);
int yng_setsockopt(int sockfd, int level, int optname,
		   const void *optval, socklen_t optlen);
int yng_accept(int sockfd,
	       struct sockaddr *cliaddr, socklen_t *addrlen);
const char *yng_getsockname(int sockfd, char *strptr);
const char *yng_getname(int sockfd, char *strptr,
			int (*funptr)(int, struct sockaddr *, socklen_t *));
const char *yng_getpeername(int sockfd, char *strptr);


#endif
