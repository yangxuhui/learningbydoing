/* This is a simple http server.
 * Author: Jimmy Young
 * young.c - An tiny HTTP server inspired by CSAPP.
 * This simple http server also uses some ideas from
 * CS162(Berkeley: https://cs162.eecs.berkeley.edu/static/hw/hw2.pdf)
 * and Tinyhttpd(https://github.com/EZLippi/Tinyhttpd).
 */


#include "young.h"


void sigchld_handler(int sig)
{
  while (waitpid(-1, 0, WNOHANG) >0)
    ;
  return;
}


int main(int argc, char **argv)
{
  char *port;
  int listen_sockfd, connect_sockfd;
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len;
  char server_name[NAMESIZE], client_name[NAMESIZE];
  pid_t pid;
  
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
  signal(SIGCHLD, sigchld_handler);

  while (1) {
    connect_sockfd = accept(listen_sockfd,
				(struct sockaddr *)&client_addr,
				&client_addr_len);
    if (connect_sockfd < 0) {
      fprintf(stderr, "accept error: %s\n", strerror(errno));
      exit(1);
    }

    printf("Accepted a connection from %s\n",
	   yng_getpeername(connect_sockfd, client_name));

    if ((pid = fork()) < 0) {
      fprintf(stderr, "fork error: %s\n", strerror(errno));
    }
    
    if (pid == 0) {
      close(listen_sockfd);
      yng_serve_connection(connect_sockfd);
      close(connect_sockfd);
      exit(0);
    }
    close(connect_sockfd);
  }
  exit(0);
}
