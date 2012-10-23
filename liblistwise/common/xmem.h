#ifndef _XMEM_H
#define _XMEM_H

#include <sys/types.h>

/// xmalloc
//
// does this: *target = calloc(size, 1);
//  recall that calloc zeroes out the memory it allocates
//
// returns nonzero on success
//  
// example:
//
//  char* memory;
//  fatal(xmalloc, &memory, 10);
//
#define xmalloc_onfail "target: 0x%08x, size: %zu"
int xmalloc(void* target, size_t size) __attribute__((nonnull));

/// xrealloc (see realloc)
//
// equivalent to: *target = realloc(*target, es * ec), EXCEPT:
// 		- ensures that any new portion of memory is zeroed out
//
// parameters
//  es  - element size
//  ec  - element count
//  oec - old element count
//
// returns
//   zero for ENOMEM, 1 otherwise
//
// example:
//
// char* memory;
// fatal(xmalloc, &memory, 10, 0);
// ...
// fatal(xrealloc, &memory, 20, 10);
//
// NOTES
//  when oec == 0, the entire allocated space is memset to zero - you still get the
//  benefit of reusing the already-allocated memory if the new request is equal
//  to or lesser than the previous request on the same address, in size
//
#define xrealloc_onfail "target: 0x%08x, es: %zu, ec: %zu, oec: %zu"
int xrealloc(void* target, size_t es, size_t ec, size_t oec) __attribute__((nonnull));

/// xfree
//
// does this:
//   free(*target);
//   *target = 0;
//
void xfree(void* target) __attribute__((nonnull));

#endif
