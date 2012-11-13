#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>
#include <listwise/object.h>

#include "control.h"

/*

rx (reflect) operator - replace selected object entries with their reflected properties

ARGUMENTS
	*prop - property name - default is the objects default string property

OPERATION

	1. if nothing selected, select all
	2. foreach selected entry
		3. if that entry is an object, replace the entry with the collection returned by its reflect method

*/

static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype				= LWOP_SELECTION_READ | LWOP_SELECTION_RESET | LWOP_ARGS_CANHAVE
	, .op_exec			= op_exec
	, .desc					= "replace object entries with their reflected properties"
};

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	char* prop = 0;
	if(o->argsl)
		prop = o->args[0]->s;

	int x;
	for(x = ls->s[0].l - 1; x >= 0; x--)
	{
		int go = 1;
		if(!ls->sel.all)
		{
			if(ls->sel.sl > (x/8))
			{
				go = (ls->sel.s[x/8] & (0x01 << (x%8)));
			}
		}

		if(go)
		{
			if(ls->s[0].s[x].type)
			{
				listwise_object * def = 0;
				fatal(listwise_lookup_object, ls->s[0].s[x].type, &def);

				if(def)
				{
					void **		r = 0;
					uint8_t *	rtypes = 0;
					int *			rls = 0;
					int				rl = 0;

					fatal(def->reflect, *(void**)ls->s[0].s[x].s, prop, &r, &rtypes, &rls, &rl);

					// make room
					int l = ls->s[0].l;
					fatal(lstack_ensure, ls, 0, x + rl - 1, sizeof(void*));

					memmove(
						  &ls->s[0].s[x + rl]
						, &ls->s[0].s[x + 1]
						, (l - x - 1) * sizeof(ls->s[0].s[0])
					);

					int i;
					for(i = 0; i < rl; i++)
					{
						if(rtypes[i])
							fatal(lstack_obj_write, ls, 0, x + i, r[i], rtypes[i]);
						else
							fatal(lstack_write, ls, 0, x + i, r[i], rls[i]);
					}
				}
			}
		}
	}

	return 1;
}
