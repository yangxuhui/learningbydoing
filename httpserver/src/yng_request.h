/* Author: Jimmy Young
 * yng_request.h
 */


#ifndef _YNG_REQUEST_
#define _YNG_REQUEST_


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define RIO_BUFSIZE 8192


extern char **environ;


typedef struct {
  int rio_fd;                    /* Descriptor for this internal buf */
  int rio_cnt;                   /* Unread bytes in internal buf */
  char *rio_bufptr;              /* Next unread byte in internal buf */
  char rio_buf[RIO_BUFSIZE];  /* Internal buffer */
} rio_t;


void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen);
ssize_t rio_writen(int fd, char *usrbuf, size_t n);


void yng_serve_connection(int connect_sockfd);
void serve_static(int fd, const char *filename, int filesize);
void serve_dynamic(int fd, char *filename, char *query_string);
void bad_request(int fd);
void unimplemented(int fd);
void not_found(int fd);
void forbidden(int fd);
void internal_server_error(int fd);
void yng_http_send_error(int fd, const char *cause, int errnum,
			 const char *shortmsg, const char *longmsg);
void yng_http_client_error_body(char *buf, const char *cause, 
				int errnum, const char *shortmsg,
				const char *longmsg);
const char* yng_http_get_response_message(int status_code);
void yng_http_start_response(char *buf, int status_code);
void yng_http_send_header(char *buf, const char *key, const char *value);
void yng_http_end_headers(char *buf);
const char *yng_http_get_mime_type(const char *file_name);

#endif
