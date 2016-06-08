/*
 * mm_alloc.h
 * Description: A clone of the interface documented in "man 3 malloc".
 * Author: Jimmy Young
 */


#include <stdlib.h>


void *mm_malloc(size_t size);
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
