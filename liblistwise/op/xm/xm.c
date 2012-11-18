#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>

#include "control.h"

/*

xm operator - Match by filename eXtension

ARGUMENTS

  0  - extension

OPERATION

	1. foreach item in selection, or, if no selection, in top list
	2. select that item if its string ends with "." extension

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype				= LWOP_SELECTION_READ | LWOP_SELECTION_WRITE | LWOP_ARGS_CANHAVE
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "select by filename extension"
};

int op_validate(operation* o)
{
	if(o->argsl != 1)
		fail("xm -- arguments : %d", o->argsl);

	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	char* xs = o->args[0]->s;
	int xl = o->args[0]->l;

	int x;
	for(x = 0; x < ls->s[0].l; x++)
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
			char * s = 0;
			int l = 0;
			lstack_string(ls, 0, x, &s, &l);

			if(l > xl)
			{
				if(s[l - xl - 1] == '.')
				{
					if(memcmp(s + (l - xl), xs, xl) == 0)
					{
						fatal(lstack_last_set, ls, x);
					}
				}
			}
		}
	}

	return 1;
}
