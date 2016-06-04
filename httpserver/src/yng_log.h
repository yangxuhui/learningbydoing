/*
 * Author: Jimmy Young
 * yng_log.h
 */


#ifndef _YNG_LOG_
#define _YNG_LOG_


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


void yng_unix_error(char *msg);
void yng_posix_error(int code, char *msg);
void yng_app_error(char *msg);


#endif
