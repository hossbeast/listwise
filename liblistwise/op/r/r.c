#include <listwise/operator.h>

/*

r operator - reverse items

NO ARGUMENTS

 use current selection, ELSE
 use entire top list

OPERATION

 1. reverse the items amongst themselves

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	r - "
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int l = 0;

	if(ls->sel.l)
	{
		int x;
		for(x = 0; x < (ls->sel.l / 2); x++)
		{
			int a = ls->sel.s[x];
			int b = ls->sel.s[ls->sel.l - 1 - x];

			typeof(ls->s[0].s[0]) t = ls->s[0].s[a];
			ls->s[0].s[a] = ls->s[0].s[b];
			ls->s[0].s[b] = t;
		}
	}
	else
	{
		int x;
		for(x = 0; x < (ls->s[0].l / 2); x++)
		{
			int a = x;
			int b = ls->s[0].l - 1 - a;

			typeof(ls->s[0].s[0]) t = ls->s[0].s[a];
			ls->s[0].s[a] = ls->s[0].s[b];
			ls->s[0].s[b] = t;
		}
	}

	return 1;
}
