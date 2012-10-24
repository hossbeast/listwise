#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include <listwise/operator.h>

/*

sn operator - sort numbers (in ascending order)

NO ARGUMENTS

 use current selection, ELSE
 use entire top list

OPERATION

 1. sort the items amongst themselves

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	sn - "
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
		l = ls->sel.l;
	}
	else
	{
		int compar(const void * _A, const void * _B)
		{
			typeof(ls->s[0].s[0]) * A = (void*)_A;
			typeof(ls->s[0].s[0]) * B = (void*)_B;

			return atoi(A->s) - atoi(B->s);
		};

		qsort(ls->s[0].s, ls->s[0].l, sizeof(ls->s[0].s[0]), compar);
	}

	return 1;
}
