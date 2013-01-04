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
	  .optype				= LWOP_SELECTION_READ | LWOP_SELECTION_WRITE
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "invert selection"
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int x;
	for(x = 0; x < ls->s[0].l; x++)
	{
		int go = 1;		// go = 1 if the item is NOT SELECTED
		if(!ls->sel.all)
		{
			if(ls->sel.sl > (x/8))
			{
				if(ls->sel.s[x/8] & (0x01 << (x%8)))	// whether it is selected
					go = 0;
			}
		}

		if(go)
			fatal(lstack_last_set, ls, x);
	}

	finally : coda;
}
