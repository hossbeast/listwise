#include <stdlib.h>
#include <alloca.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <listwise/operator.h>

#include "control.h"

/*

v operator - invert current selection

NO ARGUMENTS

OPERATION

 1. invert selectedness of each item

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	v - "
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int * sel = alloca(ls->sel.l * sizeof(sel[0]));
	memcpy(sel, ls->sel.s, ls->sel.l * sizeof(sel[0]));

	int sell = ls->sel.l;

	fatal(lstack_sel_clear, ls);

	int x;
	int z = 0;
	for(x = 0; x < ls->s[0].l; x++)
	{
		if(z >= sell || x != sel[z])
		{
			fatal(lstack_sel_write, ls, x);
		}
		else
		{
			z++;
		}
	}

	return 1;
}
