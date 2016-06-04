/*
 * Author: Jimmy Young
 * yng_log.c - Provides the functions of Young's core logging facilities.
 * Inspired by CSAPP
 */


#include "yng_log.h"


void yng_unix_error(char *msg)        /* Unix-style error */
{
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(0);
}


void yng_posix_error(int code, char *msg)        /* Posix-style error */
{
  fprintf(stderr, "%s: %s\n", msg, strerror(code));
  exit(0);
}


void yng_app_error(char *msg)        /* Application error */
{
  fprintf(stderr, "%s\n", msg);
  exit(0);
}
