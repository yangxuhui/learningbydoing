/* Author: Jimmy Young
 * yng_util.h
 */


#ifndef _YNG_UTIL_
#define _YNG_UTIL_


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>


const char *yng_getname(int sockfd, char *strptr,
			int (*funptr)(int, struct sockaddr *, socklen_t *));
const char *yng_getpeername(int sockfd, char *strptr);
const char *yng_getsockname(int sockfd, char *strptr);


#endif
