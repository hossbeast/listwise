#include <listwise/operator.h>

#include "control.h"

/*

shift operator - delete the 0th list from the stack

NO ARGUMENTS

OPERATION

 1. delete the 0th list from the stack

*/

static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype				= LWOP_SELECTION_RESET
	, .op_exec			= op_exec
	, .desc					= "delete the 0th list"
};

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	fatal(lstack_shift, ls);

	// if anything was selected, its now used up
	fatal(lstack_sel_all, ls);

	return 1;
}
