#ifndef _LISTWISE_H
#define _LISTWISE_H

#include <stdint.h>

/// list-stack
//
// stack of lists of strings
//
struct lstack;
typedef struct lstack
{
	struct	// stack
	{
		struct // list
		{
			char*		s;	// string

			int			l;	// len - number of characters
			int			a;	// alloc
		}					*s;

		int			l;	// len - number of strings
		int			a;	// alloc
	}					*s;

	int			l;	// len - number of lists
	int			a;	// alloc

	struct 	// selection
	{
		uint8_t *	s;	// bitvector of selected positions
		int				l;	// number of selected items

		int				sl;	// length of s
		int				sa;	// allocated size of s
	} sel;
} lstack;

/// listwise_exec
//
// SUMMARY
//  execute the listwise generator, receive an lstack result
//
// PARAMETERS
//  s      - generator string
//  l      - s length, or 0 for strlen
//  init   - items to write to the stack before executing
//  initls - lengths for items in init, 0 for strlen
//  initl  - number of items in init
//  r      - receives results
//
// RETURNS
//  1 for success
//
int listwise_exec(char* s, int l, char** init, int* initls, int initl, lstack** r);

/// lstack_free
//
// free lstack with free-like semantics
//
void lstack_free(lstack*);

/// lstack_xfree
//
// free an lstack with xfree-like semantics
//
void lstack_xfree(lstack**);

#endif
