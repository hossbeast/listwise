#ifndef _LISTWISE_INTERNAL_H
#define _LISTWISE_INTERNAL_H

#include "listwise.h"
#include "listwise/operator.h"
#include "listwise/generator.h"
#include "listwise/lstack.h"
#include "listwise/ops.h"

#define API __attribute__((visibility("protected")))
#define APIDATA

int op_load(char* path);
void op_sort();
void op_teardown();

/*

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

*/

#endif
