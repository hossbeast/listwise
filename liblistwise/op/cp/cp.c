#include <stdlib.h>
#include <string.h>

#include <listwise/operator.h>

#include "control.h"

/*

cp operator - duplicate list entries

ARGUMENTS

 number of copies - default : 1

OPERATION

 use current select, ELSE
 use entire top list

 for each entry
	1. create N copies of this entry at indexes x + 1 .. N

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
		fail("cp - first argument should be i64");

	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int N = 1;
	if(o->argsl)
		N = o->args[0]->i64;

	int x = 0;
	int c = 0;
	int i = 0;

	if(ls->sel.all)
		c = ls->s[0].l;
	else
	{
		for(x = 0; x < ls->s[0].l; x++)
		{
			int go = 1;
			if(ls->sel.sl <= (x/8))	// could not be selected
				break;

			if(ls->sel.s[x/8] & (0x01 << (x%8)))
				c++;
		}
	}

	// preallocate additional entries
	int k = ls->s[0].l;
	fatal(lstack_ensure, ls, 0, ls->s[0].l + (c * N) - 1, 0);

	for(x = k - 1; x >= 0; x--)
	{
		int go = 1;
		if(!ls->sel.all)
		{
			if(ls->sel.sl <= (x/8))	// could not be selected
				continue;

			go = (ls->sel.s[x/8] & (0x01 << (x%8)));	// whether it is selected
		}

		if(go)
		{
			// move following entries down to make space
			memmove(
				  &ls->s[0].s[x + N + 1]
				, &ls->s[0].s[x + 1]
				, ((k - x - 1) + i) * sizeof(ls->s[0].s[0])
			);

			int y;
			for(y = 1; y <= N; y++)
			{
				// ensure these slots are actually reallocated
				memset(&ls->s[0].s[x + y], 0, sizeof(ls->s[0].s[0]));

				// duplicate contents into new entry
				fatal(lstack_write, ls, 0, x + y, ls->s[0].s[x].s, ls->s[0].s[x].l);
			}

			i += N;
		}
	}

	// selection reset
	fatal(lstack_sel_all, ls);

	return 1;
}
