/* Author: Jimmy Young
 * yng_request.c - Provides the functions of Young's core 
 *                 request handling facilities, including
 *                 a robust I/O package.
 */


#include "yng_request.h"


void yng_serve_connection(int connect_sockfd)
{
  char request_line[1024], header_buf[1024];
  rio_t rio;
  size_t numchars;
  char method[256], url[512], path[512];
  int cgi = 0;
  char *query_string = NULL;
  size_t i, j;
  struct stat st;

  rio_readinitb(&rio, connect_sockfd);
  numchars = rio_readlineb(&rio, request_line, sizeof request_line);

  // print request line
  puts("Client request:");
  printf("%s", request_line);

  if (numchars <= 0 || request_line[numchars-1] != '\n') {
    bad_request(connect_sockfd);
    return;
  }

  i = 0;
  j = 0;
  while (!isspace(request_line[i]) && (i < sizeof(method) - 1) && i < numchars) {
    method[i] = request_line[i];
    i++;
  }

  if (i == numchars) {
    bad_request(connect_sockfd);
    return;
  }

  j = i;
  method[i] = '\0';

  if (strcasecmp(method, "GET")) {
    unimplemented(connect_sockfd);
    return;
  }

  i = 0;
  while (request_line[j] == ' ' && j < numchars)
    j++;
  while (!isspace(request_line[j]) && (i < sizeof(url) - 1) && j < numchars) {
    url[i++] = request_line[j++];
  }
  
  if (j == numchars) {
    bad_request(connect_sockfd);
    return;
  }

  url[i] = '\0';
  
  if (strcasecmp(method, "GET") == 0) {
    query_string = url;
    while (*query_string != '?' && *query_string != '\0')
      query_string++;
    if (*query_string == '?') {
      cgi = 1;
      *query_string = '\0';
      query_string++;
    }
  }

  
  // read and print request headers
  do {
    numchars = rio_readlineb(&rio, header_buf, sizeof header_buf);
    printf("%s", header_buf);
  } while (numchars > 0 && strcmp("\r\n", header_buf));
  
  sprintf(path, ".%s", url);
  if (path[strlen(path) - 1] == '/')
    strcat(path, "index.html");
  if (stat(path, &st) == -1) {
    not_found(connect_sockfd);
  } else {
    if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
      cgi = 1;
    if (!cgi) {
      if (!(S_ISREG(st.st_mode)) || !(S_IRUSR & st.st_mode)) {
	forbidden(connect_sockfd);
	return;
      }
      serve_static(connect_sockfd, path, st.st_size);
    }
    else
      serve_dynamic(connect_sockfd, path, query_string);
  }
}


void serve_static(int fd, const char *filename, int filesize)
{
  const char *filetype;
  char *srcp;
  char buf[1024], length[128];
  int file;

  filetype = yng_http_get_mime_type(filename);
  yng_http_start_response(buf, 200);
  yng_http_send_header(buf, "Server", "Young Web Server");
  sprintf(length, "%d", filesize);
  yng_http_send_header(buf, "Content-length", length);
  yng_http_send_header(buf, "Content-type", filetype);
  yng_http_end_headers(buf);

  rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  file = open(filename, O_RDONLY, 0);
  if (file < 0) {
    fprintf(stderr, "server_static error: open error: %s\n",
	    strerror(errno));
    exit(1);
  }
  srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, file, 0);
  close(file);
  rio_writen(fd, srcp, filesize);
}


void serve_dynamic(int fd, char *filename, char *query_string)
{
  char buf[1024];
  char *emptylist[] = { NULL };
  int status;

  pid_t pid;
  
  if ((pid = fork()) < 0) {
    fprintf(stderr, "serve_dynamic error: fork error: %s\n",
	    strerror(errno));
    internal_server_error(fd);
    return;
  }

  yng_http_start_response(buf, 200);
  yng_http_send_header(buf, "Server", "Young Web Server");
  rio_writen(fd, buf, strlen(buf));

  if (pid == 0) {    /* child */
    setenv("QUERY_STRING", query_string, 1);
    if (dup2(fd, STDOUT_FILENO) < 0) {
      fprintf(stderr, "serve_dynamic error: dup2 error: %s\n",
	      strerror(errno));
      internal_server_error(fd);
      return;
    }
    if (execve(filename, emptylist, environ) < 0) {
      fprintf(stderr, "serve_dynamic error: execve error: %s\n",
	      strerror(errno));
      internal_server_error(fd);
      return;
    }
  }
  waitpid(pid, &status, 0);
}


/************************************************************/
/* The following robust reading and writing package
 * is developed by Randal E. Bryant and David R. O'Hallaron
 * in CSAPP.
 */
/***********************************************************/
void rio_readinitb(rio_t *rp, int fd)
{
  rp->rio_fd = fd;
  rp->rio_cnt = 0;
  rp->rio_bufptr = rp->rio_buf;
}


ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
  size_t cnt;

  while (rp->rio_cnt <= 0) {        /* Refill if buf is empty */
    rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
		       sizeof(rp->rio_buf));
    if (rp->rio_cnt < 0) {
      if (errno != EINTR)
	return -1;
    }
    else if (rp->rio_cnt == 0)        /* EOF */
      return 0;
    else
      rp->rio_bufptr = rp->rio_buf;        /* Reset buffer ptr */
  }

  cnt = n;
  if (rp->rio_cnt < n)
    cnt = rp->rio_cnt;
  memcpy(usrbuf, rp->rio_bufptr, cnt);
  rp->rio_bufptr += cnt;
  rp->rio_cnt -= cnt;
  return cnt;
}


ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen)
{
  int n, rc;
  char c, *bufp = usrbuf;

  for (n = 1; n < maxlen; n++) {
    if ((rc = rio_read(rp, &c, 1)) == 1) {
      *bufp++ = c;
      if (c == '\n')
	break;
    } else if (rc == 0) {
      if (n == 1)
	return 0;        /* EOF, no data read */
      else
	break;
    } else {
      return -1;        /* Error */
    }
  }
  *bufp = '\0';
  return n;
}


ssize_t rio_writen(int fd, char *usrbuf, size_t n)
{
  size_t nleft = n;
  ssize_t nwritten;
  char *bufp = usrbuf;

  while (nleft > 0) {
    if ((nwritten = write(fd, bufp, nleft)) <= 0) {
      return -1;
    }
    nleft -= nwritten;
    bufp += nwritten;
  }
  return n;
}


void bad_request(int fd)
{
  yng_http_send_error(fd, "malformed request",
			     400, "Bad request", 
			     "You sent a malformed request");
}

void unimplemented(int fd)
{
  yng_http_send_error(fd, "you sent an unimplemented method",
		      501, "Not Implemented",
		      "Young has not implemented the requested method");
}

void not_found(int fd)
{
  yng_http_send_error(fd, "requested file not found",
		      404, "Not Found",
		      "Young can't find the requested file");
}

void forbidden(int fd)
{
  yng_http_send_error(fd, "you are not allowed",
		      403, "Forbidden", "Young couldn't handle the request");
}

void internal_server_error(int fd)
{
  yng_http_send_error(fd, "Sorry, Young encounters an error",
		      500, "Internal Server Error",
		      "Young encounters an error that prevents it from servicing the request");
}

/************************************************************/
/* Returns an error message to the client.
 * Parameters: fd: client socket 
 *             cause: error reason
 *             errnum: status code
 *             shortmsg: reason phrase
 *             longmsg: description of the error
 */
/************************************************************/
void yng_http_send_error(int fd, const char *cause, int errnum,
			 const char *shortmsg, const char *longmsg)
{
  char buf[1024], body[8192], length[16];

  yng_http_client_error_body(body, cause, errnum, shortmsg, longmsg);
  
  yng_http_start_response(buf, errnum);
  yng_http_send_header(buf, "Server", "Young Web Server");
  sprintf(length, "%d", (int)strlen(body));
  yng_http_send_header(buf, "Content-length", length);
  yng_http_send_header(buf, "Content-type", "text/html");
  yng_http_end_headers(buf);

  rio_writen(fd, buf, strlen(buf));
  rio_writen(fd, body, strlen(body));
}

void yng_http_client_error_body(char *buf, const char *cause, 
				int errnum, const char *shortmsg,
				const char *longmsg)
{
  sprintf(buf, "<html><title>Young Error</title>");
  sprintf(buf, "%s<body><p>%d: %s</p>", buf, errnum, shortmsg);
  sprintf(buf, "%s<p>%s: %s</p>", buf, longmsg, cause);
  sprintf(buf, "%s<hr><em>Young Web Server</em></body></html>", buf);
}

const char* yng_http_get_response_message(int status_code) 
{
  switch (status_code) {
    case 100:
      return "Continue";
    case 200:
      return "OK";
    case 301:
      return "Moved Permanently";
    case 302:
      return "Found";
    case 304:
      return "Not Modified";
    case 400:
      return "Bad Request";
    case 401:
      return "Unauthorized";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    default:
      return "Internal Server Error";
  }
}

void yng_http_start_response(char *buf, int status_code) 
{
  sprintf(buf, "HTTP/1.0 %d %s\r\n", status_code,
      yng_http_get_response_message(status_code));
}

void yng_http_send_header(char *buf, const char *key, const char *value) 
{
  sprintf(buf, "%s%s: %s\r\n", buf, key, value);
}

void yng_http_end_headers(char *buf) 
{
  sprintf(buf, "%s\r\n", buf);
}


const char *yng_http_get_mime_type(const char *file_name) 
{
  char *file_extension = strrchr(file_name, '.');
  if (file_extension == NULL) {
    return "text/plain";
  }

  if (strcmp(file_extension, ".html") == 0 || strcmp(file_extension, ".htm") == 0) {
    return "text/html";
  } else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0) {
    return "image/jpeg";
  } else if (strcmp(file_extension, ".png") == 0) {
    return "image/png";
  } else if (strcmp(file_extension, ".css") == 0) {
    return "text/css";
  } else if (strcmp(file_extension, ".js") == 0) {
    return "application/javascript";
  } else if (strcmp(file_extension, ".pdf") == 0) {
    return "application/pdf";
  } else {
    return "text/plain";
  }
}
