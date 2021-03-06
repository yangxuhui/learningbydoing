/* Author: Jimmy Young
 * yng_util.c
 */


#include "yng_util.h"


/***********************************************************/
/* yng_getsockname
 * sockfd: socket descriptor
 * strptr: character array to store the socket name
 * return a pointer the first character of the socket name
 * if success, else print the error message on the stderr
 * and exit the program.
 * The socket name is in the form
 * (X.X.X.X, port) for IPv4 and coressponding for IPv6
 */
/**********************************************************/
const char *yng_getsockname(int sockfd, char *strptr)
{
  const char *ret;
  
  if ((ret = yng_getname(sockfd, strptr, getsockname)) == NULL) {
    fprintf(stderr, "yng_getsockname error: cannot get name\n");
    exit(1);
  }
  return ret;
}


/**********************************************************/
/* yng_getpeername
 * same as yng_getsockname but get the peer socket name
 */
/*********************************************************/
const char *yng_getpeername(int sockfd, char *strptr)
{
  const char *ret;

  if ((ret = yng_getname(sockfd, strptr, getpeername)) == NULL) {
    fprintf(stderr, "yng_getpeername error: cannot get name\n");
    exit(1);
  }
  return ret;
}


/*************************
 * helper function for 
 * yng_getsockname and
 * yng_getpeername
 *************************/

/*********************************************************/
/* yng_getname
 * sockfd: socket descriptor
 * strptr: character array to store the socket name
 * funptr: a pointer to function that are etheir 
 *         getsockname or getpeername
 * return a pointer the first character of the socket name
 * if success, else return NULL.
 * The socket name is in the form
 * (X.X.X.X, port) for IPv4 and coressponding for IPv6
 */
/********************************************************/
const char *yng_getname(int sockfd, char *strptr,
			int (*funptr)(int, struct sockaddr *, socklen_t *))
{
  char port[20];
  struct sockaddr_storage localaddr;
  socklen_t addrlen;
  char *fun_name = "yng_getname";

  addrlen = sizeof(localaddr);
  if ((*funptr)(sockfd, (struct sockaddr *)&localaddr, &addrlen) < 0) {
    fprintf(stderr, 
	    "%s error: getsockname/getpeername failed: %s\n",
	    fun_name, strerror(errno));
    exit(1);
  }

  strptr[0] = '(';
  switch (localaddr.ss_family) {
  case AF_INET: {
    struct sockaddr_in *sin = (struct sockaddr_in *)&localaddr;

    if (inet_ntop(AF_INET, &sin->sin_addr, strptr + 1,
		  INET_ADDRSTRLEN) == NULL)
    {
      fprintf(stderr, "%s error: inet_ntop failed: %s\n",
	      fun_name, strerror(errno));
      exit(1);
    }
  
    if (ntohs(sin->sin_port) != 0) {
      snprintf(port, sizeof(port), ", %d)", ntohs(sin->sin_port));
      strcat(strptr, port);
    }
    return strptr;
  }
  case AF_INET6: {
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&localaddr;

    if (inet_ntop(AF_INET6, &sin6->sin6_addr, strptr + 1,
		  INET6_ADDRSTRLEN) == NULL)
    {
      fprintf(stderr, "%s error: inet_ntop failed: %s\n",
	      fun_name, strerror(errno));
      exit(1);
    }
    if (ntohs(sin6->sin6_port) != 0) {
      snprintf(port, sizeof(port), ", %d)", ntohs(sin6->sin6_port));
      strcat(strptr, port);
    }
    return strptr;
  }
  default:
    fprintf(stderr, "%s error: Unsupported socket family\n", fun_name);
    exit(1);
  }
  return NULL;
}
