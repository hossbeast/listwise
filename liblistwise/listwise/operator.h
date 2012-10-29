#ifndef _LISTWISE_OPERATOR_H
#define _LISTWISE_OPERATOR_H

#include <stdint.h>
#include <stdio.h>

#include <pcre.h>

#include <listwise.h>

#define restrict __restrict

/*
** LISTWISE OPERATOR API
**
*/

struct operation;
struct operator;
struct arg;

//
// operators are of one of these types
//
enum
{
	  LWOP_SELECTION_READ					= 0x0001			// uses the current selection
	, LWOP_SELECTION_WRITE				= 0x0002			// implicitly apply the y operator after this operator
	, LWOP_SELECTION_RESET				= 0x0004			// resets the current selection
	, LWOP_MODIFIERS_CANHAVE			= 0x0008			// last argument is a modifiers string
	, LWOP_ARGS_CANHAVE						= 0x0010			// can have arguments
	, LWOP_OPERATION_PUSHBEFORE		= 0x0020			// first operation is to push an empty list
	, LWOP_OPERATION_INPLACE			= 0x0040			// modifies the string in the top list in-place
	, LWOP_OPERATION_FILESYSTEM		= 0x0080			// filesystem operator
};

//
// operator - each op exports one of these structs
//
typedef struct operator
{
	void *		handle;		// dlopen handle
	uint64_t	optype;		// OPTYPE_*
	char*			desc;			// operator description

	// methods
	int 		(*op_validate)(struct operation*);
	int 		(*op_exec)(struct operation*, lstack*, int**, int*);

	char		s[6];			// name; overwritten by liblist during initialization
	int			sl;				// name length
} operator;

//
// references which occur within arguments are of one of these types
//
enum
{
	REFTYPE_BREF = 10
};

//
// argument
//
typedef struct arg
{
	char*		s;		// string value of the arg, null-terminated
	int			l;		// string length

	struct ref		// references within the string
	{
		char* s;		// start of the reference 
		char* e;		//   end of the reference (pointer to the character following the last character of the reference)
		int		l;		// length of the reference (l = e - s)

		int		k;		// REFTYPE_*

		union
		{
			int		bref;		// for REFTYPE_BREF, value of the backreference
		};
	}				*refs;
	int			refsl;	// number of references

	// pointer to the last reference, if any
	struct ref* ref_last;

#define ITYPE_RE		1
#define ITYPE_I64		2

	// indicates which member of the interpreted value union to use
	uint8_t						itype;

	union					// interpreted value of the arg, if any
	{
		struct re
		{
			pcre*					c_pcre;
			pcre_extra*		c_pcre_extra;
			int						c_caps;
		}								re;

		int64_t					i64;
	};
} arg;

//
// an operation consists of an operator and a set of arguments
//
typedef struct operation
{
	struct operation*	next;

	operator*					op;			// operator
	arg**							args;		// arguments
	int								argsl;
} operation;

/// lstack_append
//
// append text to the entry at x:y
//
int lstack_append(lstack * const restrict ls, int x, int y, const char* const restrict s, int l)
	__attribute__((nonnull));

/// lstack_appendf
//
// append text to the entry at x:y using printf-style args
//
int lstack_appendf(lstack* const restrict ls, int x, int y, const char* const restrict s, ...)
	__attribute__((nonnull));

/// lstack_write
//
// write text to the entry at x:y
//
int lstack_write(lstack* const restrict ls, int x, int y, const char* const restrict s, int l)
	__attribute__((nonnull));

/// lstack_writef
//
// write text to the entry at x:y using printf-style args
//
int lstack_writef(lstack* const restrict ls, int x, int y, const char* const restrict s, ...)
	__attribute__((nonnull));

/// lstack_add
//
// write text to the next unused entry of list 0
//
int lstack_add(lstack* const restrict ls, const char* const restrict s, int l)
	__attribute__((nonnull));

/// lstack_addf
//
// write text to the next unused entry of list 0 using printf-style args
//
int lstack_addf(lstack* const restrict ls, const char* const restrict fmt, ...)
	__attribute__((nonnull));

/// lstack_push
//
// push an empty list onto the stack
//
int lstack_push(lstack* const restrict ls)
	__attribute__((nonnull));

/// lstack_cycle
//
// move the first list on the stack to the last position
//
int lstack_cycle(lstack* const restrict ls)
	__attribute__((nonnull));

/// lstack_recycle
//
// move the last list on the stack to the first position
//
int lstack_recycle(lstack* const restrict ls)
	__attribute__((nonnull));

/// lstack_xchg
//
// exchange the first and second lists on the stack
//
int lstack_xchg(lstack* const restrict ls)
	__attribute__((nonnull));

/// sel_clear
//
// clear the selection
//
int lstack_sel_clear(lstack* const restrict ls)
	__attribute__((nonnull));

/// sel_set
//
// select the entry at 0:y
//
int lstack_sel_set(lstack* const restrict ls, int y)
	__attribute__((nonnull));

/// sel_write
//
// select exactly those entries specified in news
//
// PARAMETERS
//  ls    - list-stack
//  news  - bitvector specifying the selection
//  newsl - elements in news
//
int lstack_sel_write(lstack* const restrict ls, uint8_t * news, int newsl)
	__attribute__((nonnull));

/// ensure
//
// ensure stack/list allocation up to the specified dimensions
//
// parameters
//   x - zero based list index       ( or -1 to skip stack allocation)
//   y - zero based string index     ( or -1 to skip list allocation)
//   z - zero based character index  ( or -1 to skip string allocation)
//
// updates stack and list length to be at least the dimension specified
//  ** this is NOT done for string length **
//
int lstack_ensure(lstack* const restrict ls, int x, int y, int z);

/// re_compile
//
// compile the re for an op arg representing a regex
//
// parameters
//  re  - regex op arg
//  mod - modifier string arg, if any
//
int re_compile(char* s, struct re* re, char* mod);

/// re_exec
//
// execute an regex against a string
//
// parameters
//  re       - regex already compiled with re_compile
//  s        - subject string
//  l        - length of s
//  o        - offset from s to start matching
//  ovec     - results and subcapture information written to *ovec
//  ovec_len - *ovec size is at *ovec_len
//
// returns 1 on success, 0 on failure (memory allocation, bad pcre, etc)
//
// example
//    char* s;
//    int l;
//    int* ovec;
//    int ovec_len;
//		if(re_exec(re, s, l, 0, &ovec, &ovec_len) == 0)
//		{
//			/* failure */
//		}
//		else if(ovec[0] > 0)
//		{
//			int x;
//			for(x = 0; x < ovec[0]; x++)
//			{
//				int a = ovec[1 + (x * 2) + 0];
//				int b = ovec[1 + (x * 2) + 1];
//
//				if(a >= 0 && b >= 0)
//				{
//					/* print x'th subcapture - recall the 0th subcapture is the entire match */
//					printf("%.*s", b - a, s + a);
//				}
//			}
//		}
//		else
//		{
//			/* no match */
//		}
//
int re_exec(struct re* re, char* s, int l, int o, int** ovec, int* ovec_len);

#endif
