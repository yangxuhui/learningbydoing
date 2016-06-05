/* This is a simple http server.
 * Author: Jimmy Young
 * young.c - An tiny HTTP server inspired by CSAPP.
 * This simple http server also uses some ideas from
 * CS162(Berkeley: https://cs162.eecs.berkeley.edu/static/hw/hw2.pdf)
 * and Tinyhttpd(https://github.com/EZLippi/Tinyhttpd).
 */


#include "young.h"


void *thread(void *vargp)
{
  int connfd = *((int *)vargp);
  pthread_detach(pthread_self());
  free(vargp);
  yng_serve_connection(connfd);
  close(connfd);
  return NULL;
}


int main(int argc, char **argv)
{
  char *port;
  int listen_sockfd, *connect_sockfd_ptr;
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len;
  char server_name[NAMESIZE], client_name[NAMESIZE];
  pthread_t tid;
  
  if (argc != 2) {
    fprintf(stderr, "usage: young <port>\n");
    exit(1);
  }

  port = argv[1];
  client_addr_len = sizeof(client_addr);
  listen_sockfd = yng_open_listening_sockets(port);
  printf("Listening at: %s\n",
	 yng_getsockname(listen_sockfd, server_name));

  signal(SIGPIPE, SIG_IGN);

  while (1) {
    connect_sockfd_ptr = malloc(sizeof(int));
    *connect_sockfd_ptr = accept(listen_sockfd,
				(struct sockaddr *)&client_addr,
				&client_addr_len);
    if (*connect_sockfd_ptr < 0) {
      fprintf(stderr, "accept error: %s\n", strerror(errno));
      exit(1);
    }

    printf("Accepted a connection from %s\n",
	   yng_getpeername(*connect_sockfd_ptr, client_name));

    pthread_create(&tid, NULL, thread, connect_sockfd_ptr);
  }
  exit(0);
}
