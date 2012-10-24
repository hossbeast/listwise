#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <listwise/operator.h>

#include "control.h"

#include "parseint.h"

/*

u operator - select stringwise-unique entries (expects an already-sorted list)

NO ARGUMENTS

 use current selection, ELSE
 use entire top list

OPERATION

 1. select entries which are unique

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	ss - "
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
	}
	else
	{
		int x;
		for(x = 0; x < ls->s[0].l; x++)
		{
			if(x == 0 || strcmp(ls->s[0].s[x-1].s, ls->s[0].s[x].s))
			{
				fatal(lstack_sel_write, ls, x);
			}
		}
	}

	return 1;
}
