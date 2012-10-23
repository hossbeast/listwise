#ifndef _LIST_INTERNAL_H
#define _LIST_INTERNAL_H

#include <stdint.h>

#include <pcre.h>

#include "listwise_op.h"

#include "control.h"

#define API __attribute__((visibility("protected")))
#define APIDATA

struct generator_t
{
	arg **				args;		// initial list contents
	int						argsl;

	operation **	ops;		// operator/args pairs to be executed sequentially
	int						opsl;
};

/// generator_dump
//
// print a generator to stdout
//
void generator_dump(generator*);

//
// operator names are at op-STR+3 (STR being the first token in generator.y)
//
static __attribute__((weakref, alias("yytname"))) const char* const list_opnames[];

//
// get the operator name for the specified operator code
//
// returns length of the name followed by the name (suitable for printf("%.*s", OPNAME(op))
//
#define OPNAME(x) (int)strlen(list_opnames[x - STR + 3]) - 2, list_opnames[x - STR + 3] + 1

// available operators
extern operator** 		g_ops;
extern int						g_ops_l;

operator*			op_lookup(char*, int);

#endif
