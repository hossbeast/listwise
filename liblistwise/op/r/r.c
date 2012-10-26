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

void swap(lstack* ls, int a, int b)
{
	typeof(ls->s[0].s[0]) t = ls->s[0].s[a];
	ls->s[0].s[a]						= ls->s[0].s[b];
	ls->s[0].s[b]						= t;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int l = 0;

	if(ls->sel.l)
	{
		int a = 0;
		while((ls->sel.s[a/8] & (0x01 << (a%8))) == 0)
			a++;

		int b = (ls->sel.sl * 8) + 7;
		while((b/8) >= ls->sel.sl || (ls->sel.s[b/8] & (0x01 << (b%8))) == 0)
			b--;

		int x;
		for(x = 0; x < (ls->sel.l / 2); x++)
		{
			swap(ls, a++, b--);

			while((ls->sel.s[a/8] & (0x01 << (a%8))) == 0)
				a++;

			while((ls->sel.s[b/8] & (0x01 << (b%8))) == 0)
				b--;
		}
	}
	else
	{
		int x;
		for(x = 0; x < (ls->s[0].l / 2); x++)
			swap(ls, x, ls->s[0].l - 1 - x);
	}

	return 1;
}
