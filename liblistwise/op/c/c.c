#include <listwise/operator.h>

#include "control.h"

/*

c operator - coalesce lists on the stack

NO ARGUMENTS

OPERATION

 1. coalesce all lists onto the top list
 2. clear the current selection

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	c - "
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int x;
	for(x = ls->l - 1; x >= 1; x--)
		fatal(lstack_merge, ls, 0, x);

	fatal(lstack_sel_clear, ls);
	return 1;
}
