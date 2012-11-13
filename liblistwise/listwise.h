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
			char*		s;		// string

			int			l;		// len - number of characters
			int			a;		// alloc

			uint8_t	type;	// object type - ignore unless using the list-object interface
		}					*s;

		int			l;	// len - number of strings
		int			a;	// alloc
	}					*s;

	int			l;	// len - number of lists
	int			a;	// alloc

	struct					// items affected by the last operation - on the top list
	{
		uint8_t *	s;	// bitvector
		int				l;	// number of bits set in s
		int				sl;	// length of s
		int				sa;	// allocated size of s
	} last;

	struct 						// currently selected items - on the top list
	{
		uint8_t *	s;		// bitvector
		int				l;		// number of bits set in s
		int				sl;		// length of s
		int				sa;		// allocated size of s

		char			all;	// if true, all items are selected - otherwise, exactly those items
										// as specified in s comprise the current selection
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

/// listwise_err_fd
//
// listwise operators write errors to this fd (ls a nonexistent path, for example)
//
// default value : 2
//
int listwise_err_fd;

#endif
