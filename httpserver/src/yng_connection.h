/* Author: Jimmy Young
 * yng_connection.h
 */


#ifndef _YNG_CONNECTION_
#define _YNG_CONNECTION_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <error.h>


#define LISTENQ 1024


int yng_open_listening_sockets(const char *port);


#endif
