#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>

#include "control.h"

/*

xs operator - eXtension Substitution

ARGUMENTS
	[0] - matching extension string
	 1  - replacement extension string

OPERATION

	1. if nothing selected, select all
	2. if a matching extension string is given, select only those entries with that extension
	2. foreach selected string
		3. replace the portion of the string following the first '.' with '.' extension

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype				= LWOP_SELECTION_READ | LWOP_ARGS_CANHAVE | LWOP_OPERATION_INPLACE | LWOP_OBJECT_NO
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "substitution by filename extension"
};

int op_validate(operation* o)
{
	if(o->argsl > 2)
		fail("xs -- arguments : %d", o->argsl);

	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	char * mxs = 0;
	int mxl = 0;
	char * rxs = 0;
	int rxl = 0;

	if(o->argsl == 2)
	{
		rxs = o->args[1]->s;
		rxl = o->args[1]->l;
		mxs = o->args[0]->s;
		mxl = o->args[0]->l;
	}
	if(o->argsl == 1)
	{
		rxs = o->args[0]->s;
		rxl = o->args[0]->l;
	}
	
	int x;
	for(x = 0; x < ls->s[0].l; x++)
	{
		int go = 1;
		if(!ls->sel.all)
		{
			if(ls->sel.sl <= (x/8))
				break;

			go = (ls->sel.s[x/8] & (0x01 << (x%8)));
		}

		if(go && ls->s[0].s[x].type == 0 && ls->s[0].s[x].l)
		{
			char * s = ls->s[0].s[x].s + ls->s[0].s[x].l;
			if(s != ls->s[0].s[x].s)
			{
				s--;
				while(s != ls->s[0].s[x].s && s[0] != '/')
					s--;
			}

			if(s != ls->s[0].s[x].s + ls->s[0].s[x].l)
			{
				s++;
				while(s != ls->s[0].s[x].s + ls->s[0].s[x].l && s[0] != '.')
					s++;
			}

			if(s[0] == '.')
			{
				int xl = ls->s[0].s[x].l - (s - ls->s[0].s[x].s) - 1;
				if(mxs)
				{
					if(mxl != xl || memcmp(s + 1, mxs, mxl))
						continue;
				}

				int newl = (s - ls->s[0].s[x].s) + 1 + rxl;
				fatal(lstack_ensure, ls, 0, x, newl);
				sprintf(s, ".%.*s", rxl, rxs);
				ls->s[0].s[x].l = newl;
				ls->s[0].t[x].w = 0;

				fatal(lstack_last_set, ls, x);
			}
		}
	}

	return 1;
}
