/*
 * Description: a simple memory allocator, in particular
 * implementations of the malloc, free, and realloc functions.
 * Author: Jimmy young
 */


#include "mm_alloc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


/* Basic constants */
#define NALLOC 1024                              /* Minimum number of units to extend heap */


typedef long Align;                              /* For alignment to long boundary */

typedef union header {                           /* Block header */
  struct {
    union header *next;                          /* Next block if on free list */
    size_t size;                                 /* Size of the block */
  } s;

  Align x;                                       /* Force alignment of blocks */

} Header;


/* Private global variables */
static Header *freeptr = NULL;                   /* Start of free list */


/* Function prototypes for internal helper routines */
static bool mm_init(void);
static bool extend_heap(size_t units);


void *mm_malloc(size_t size)
{
  Header *prevptr;
  Header *ptr;
  size_t units;

  /* Ignore spurious requests */
  if (size == 0)
  {
    return NULL;
  }

  /* Initialize the heap */
  if (freeptr == NULL)
  {
    if (!mm_init())
    {
      return NULL;
    }
  }

  units = (size + sizeof(Header) -1) / sizeof(Header) + 1;
  prevptr = freeptr;
  for (ptr = prevptr->s.next; ; prevptr = ptr, ptr = ptr->s.next)
  {
    if (ptr->s.size >= units)
    {
      if (ptr->s.size == units)
      {
	prevptr->s.next = ptr->s.next;
      }
      else
      {
	ptr->s.size -= units;
	ptr += ptr->s.size;
	ptr->s.size = units;
      }
      freeptr = prevptr;
      return ptr + 1;
    }

    if (ptr == freeptr)                          /* Wrapped around free list */ 
    {
      if (!extend_heap(units))
      {
	return NULL;
      }
      else
      {
	ptr = freeptr;
      }
    }
  }
}

void *mm_realloc(void *ptr, size_t size)
{
  Header *bp;
  Header *np;
  size_t units;

  if (ptr == NULL)
  {
    return mm_malloc(size);
  }

  if (size == 0)
  {
    mm_free(ptr);
    return NULL;
  }

  units = (size + sizeof(Header) - 1) / sizeof(Header) + 1;
  bp = (Header *)ptr - 1;
  if (bp->s.size >= units)
  {
    if (bp->s.size > units)
    {
      bp->s.size = units;
      mm_free(bp + bp->s.size + 1);
    }
    return bp + 1;
  }
  else
  {
    np = (Header *)mm_malloc(size);
    if (np == NULL)
    {
      return NULL;
    }
    
    memcpy(np, ptr, size);
    mm_free(ptr);
    return np;
  }
}

void mm_free(void *ptr)
{
  Header *p;
  Header *bp;

  /* Do nothing if passed a NULL pointer */
  if (ptr == NULL)
  {
    return;
  }

  bp = (Header *)ptr - 1;
  for (p = freeptr; !(bp > p && bp < p->s.next); p = p->s.next)
  {
    if ((p >= p->s.next) && ((bp > p) || (bp < p->s.next)))
    {
      break;                                     /* Freed block at start or end of the circled list */
    }
  }
  
  /* Coalescing blocks */
  if (bp + bp->s.size == p->s.next)              /* Join to upper block */
  {
    bp->s.size += p->s.next->s.size;
    bp->s.next = p->s.next->s.next;
  }
  else
  {
    bp->s.next = p->s.next;
  }

  if ((p->s.size != 0) && (p + p->s.size == bp)) /* Join to lower block */ 
  {
    p->s.size += bp->s.size;
    p->s.next = bp->s.next;
  }
  else
  {
    p->s.next = bp;
  }
  freeptr = p;
}


static bool mm_init(void)
{
  /* Create prologue header */
  freeptr = (Header *)sbrk(sizeof(Header));
  if (freeptr == (Header *)-1)
  {
    freeptr = NULL;
    return false;
  }
  freeptr->s.size = 0;
  freeptr->s.next = freeptr;

  if (!extend_heap(NALLOC))
  {
    freeptr = NULL;
    return false;
  }
  return true;
}

static bool extend_heap(size_t units)
{
  Header *bp;

  if (units < NALLOC)
  {
    units = NALLOC;
  }

  bp = (Header *)sbrk(units * sizeof(Header));
  if (bp == (Header *)-1)                        /* No space at all */       
  {
    return false;
  }
  bp->s.size = units;
  mm_free(bp + 1);
  return true;
}
