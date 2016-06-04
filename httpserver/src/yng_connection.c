/* Author: Jimmy Young
 * yng_connection.c - Provides the functions of Young's connection facilities.
 */


#include "yng_connection.h"


/**********************************************************/
/* Opens and returns a listening sockets that is ready 
 * to receive connection requests on the well-know port.
 * If an error occured, print the error message on the stderr
 * and exit the program.
 */
/*********************************************************/
int yng_open_listening_sockets(const char *port)
{
  int listen_sockfd;
  int error_code;
  const int optval = 1;
  struct addrinfo hints, *result, *p;
  char *fun_name = "yng_open_listening_sockets";

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;

  if ((error_code = getaddrinfo(NULL, port,
				&hints, &result)) != 0)
  {
    fprintf(stderr,
	    "%s error: getaddrinfo failed (port %s): %s\n",
	    fun_name, port, gai_strerror(error_code));
    exit(1);
  }

  for (p = result; p != NULL; p = p->ai_next) {
    listen_sockfd =
      socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listen_sockfd < 0)
      continue;

    if ((error_code = setsockopt(listen_sockfd,
				 SOL_SOCKET, SO_REUSEADDR,
				 (const void *)&optval,
				 sizeof(optval))) != 0)
    {
      fprintf(stderr, 
	      "%s error: setsockopt failed: %s\n",
	      fun_name, strerror(error_code));
      exit(1);
    }
    
    if (bind(listen_sockfd, p->ai_addr, p->ai_addrlen) == 0) 
      break;

    if (close(listen_sockfd) < 0)
    {
      fprintf(stderr, 
	      "%s error: close failed to close unbinded socket",
	      fun_name);
      exit(1);
    }
  }

  freeaddrinfo(result);
  if (p == NULL) {
    fprintf(stderr,
	    "%s error: failed to bind any socket\n", fun_name);
    exit(1);
  }

  if (listen(listen_sockfd, LISTENQ) < 0) {
    if (close(listen_sockfd) < 0) {
      fprintf(stderr,
	      "%s error: close failed to close listen_sockfd after listen failed\n",
	      fun_name);
      exit(1);
    }
    
    fprintf(stderr,
	    "%s error: listen failed\n", fun_name);
    exit(1);
  }
  return listen_sockfd;
}
