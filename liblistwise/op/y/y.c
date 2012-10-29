#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>

#include "control.h"

/*

y operator - select those items affected by the last operation

NO ARGUMENTS

OPERATION

	1. select those items affected by the preceeding operator

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype				= LWOP_SELECTION_RESET
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	y - "
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	if(ls->last.l == ls->s[0].l)
	{
		ls->sel.all = 1;
	}
	else
	{
		if(ls->sel.sa < ls->last.sl)
		{
			fatal(xrealloc, &ls->sel.s, sizeof(*ls->sel.s), ls->last.sl, ls->sel.sa);
			ls->last.sa = ls->last.sl;
		}

		memcpy(
				ls->sel.s
			, ls->last.s
			, ls->last.sl * sizeof(ls->sel.s[0])
		);
		ls->sel.l = ls->last.l;
		ls->sel.sl = ls->last.sl;
		ls->sel.all = 0;
	}

	return 1;
}
