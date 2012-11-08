#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>

#include "control.h"

/*

d operator - sunDer selected (Delete)

NO ARGUMENTS

OPERATION

	1. push an empty list on the stack
	2. invert the current selection
	3. move all selected items to the top list

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype					= LWOP_SELECTION_RESET
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "extract selected items into a new list"
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	fatal(lstack_unshift, ls);

	if(ls->sel.all || ls->sel.l == ls->s[1].l)
	{
		typeof(ls->s[0]) T = ls->s[0];
		ls->s[0] = ls->s[1];
		ls->s[1] = T;
	}
	else
	{
		fatal(lstack_ensure, ls, 0, ls->s[1].l - ls->sel.l - 1, -1);

		int i = 0;
		int x;
		for(x = ls->s[1].l - 1; x >= 0; x--)
		{
			if(ls->sel.sl > (x/8))
			{
				if((ls->sel.s[x/8] & (0x01 << (x%8))) == 0)
				{
					ls->s[0].s[i++] = ls->s[1].s[x];

					memmove(
						  &ls->s[1].s[x]
						, &ls->s[1].s[x+1]
						, (ls->s[1].l - x - 1) * sizeof(ls->s[0].s[0])
					);
				}
			}
		}

		ls->s[1].l -= i;
		ls->s[1].a -= i;
	}

	fatal(lstack_sel_all, ls);
}
