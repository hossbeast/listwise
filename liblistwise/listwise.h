#ifndef _LIST_H
#define _LIST_H

/*

SELECTION OPERATORS
	m//  - select items matching the regex
  
  
GENERAL OPERATORS
	s/// - regex substition
	e/   - delete selected
	eo/  - delete other

*/

///
/// [[ structures ]]
///

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
		int		l;
		int		a;
		int *	s;
	} sel;
} lstack;

/// generator declaration - definition is private
struct generator_t;
typedef struct generator_t generator;

/// generator parser declaration
typedef void generator_parser;

/// variable_binder
//
// invoked during lstack_exec to resolve variable bindings
//
// parameters
//  v - variable name
//  r - receives list
//  l - receives list length
//
// returns 1 if the variable has a binding
//
typedef int (*variable_binder)(char* v, char*** r, int* rl);

///
/// [[ GENERATOR API ]]
///

/// generator_mkparser
//
// returns a generator parser
//
int generator_mkparser(generator_parser** p);

/// generator_parse
//
// parse a generator string
//
// parameters
//
//   p  - parser returned from mkparser, or 
//   s  - generator string
//   l  - length of generator string, or 0 for strlen
//   r  - receives parsed generator
//
// returns zero on failure
//
int generator_parse(generator_parser* p, char* s, int l, generator** r);

/// generator_freeparser 
//
// frees a generator parser returned from mkparser
//
void generator_parser_free(generator_parser*);
void generator_parser_xfree(generator_parser**);

/// generator_dump
//
// print a generator to stdout
//
void generator_dump(generator*);

/// generator_free
//
// frees a generator returned from parse
//
void generator_free(generator*);
void generator_xfree(generator**);

///
/// [[ LSTACK API ]]
///

/// lstack_exec
//
// if *r == 0, create it as the empty list stack
//
// then, executes the generator against *r
//
// parameters
//  g   - generator
//  c   - callout
//  r   - input/output list stack
//
int lstack_exec(generator* g, variable_binder c, lstack** r);

/// lstack_dump
//
// print a list-stack to stdout
//
void lstack_dump(lstack*);

/// lstack_reset
//
// reset (but do not deallocate) a list stack
//
// no-op with zero-valued parameter
//
void lstack_reset(lstack*);

/// lstack_free
//
// free lstack with free-like semantics
//
void lstack_free(lstack*);
void lstack_xfree(lstack**);

///
/// [[ LIST API ]]
///

///
//
// when an api fails, before returning zero, it prints an error to this file descriptor
// which defaults to stderr
//
extern int list_err_fd;

#endif
