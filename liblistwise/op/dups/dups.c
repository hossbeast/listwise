#include <stdlib.h>
#include <string.h>

#include <listwise/operator.h>

#include "control.h"

/*

dups operator - duplicate list entries

ARGUMENTS

 number of copies - default : 1

OPERATION

 use current select, ELSE
 use entire top list

 for each entry
  1. create N new duplicate entries, at contiguous indexes

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype				= LWOP_SELECTION_READ | LWOP_SELECTION_RESET | LWOP_ARGS_CANHAVE
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "duplicate list entries"
};

int op_validate(operation* o)
{
	if(o->argsl >= 1 && o->args[0]->itype != ITYPE_I64)
		fail("dups - first argument should be i64");

	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int N = 1;
	if(o->argsl)
		N = o->args[0]->i64;

	int x;
	for(x = ls->s[0].l - 1; x >= 0; x--)
	{
		int go = 1;
		if(!ls->sel.all)
		{
			if(ls->sel.sl <= (x/8))	// could not be selected
				break;

			go = (ls->sel.s[x/8] & (0x01 << (x%8)));	// whether it is selected
		}

		if(go)
		{
			fatal(lstack_ensure, ls, 0, x + N, 0);

			memmove(
				  &ls->s[0].s[x + N + 1]
				, &ls->s[0].s[x + 1]
				, (ls->s[0].l - N - 1) * sizeof(ls->s[0])
			);

			int y;
			for(y = 0; y < N; y++)
				fatal(lstack_write, ls, 0, x + y, ls->s[0].s[x].s, ls->s[0].s[x].l);
		}
	}

	// selection reset
	fatal(lstack_sel_all, ls);

	return 1;
}
