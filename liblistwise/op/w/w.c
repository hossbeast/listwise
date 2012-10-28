#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>

#include "control.h"

/*

w operator - replace selected strings with a window on themselves

ARGUMENTS
	 1  - offset to start of window
		    if negative, interpreted as offset from the end of the string
		    default : 0
	*2  - length of window
		    default : 0 - entire string
	*3  - modifiers
				y - replace the current selection with those items affected by w

OPERATION

	1. if nothing selected, select all
	2. foreach selected string
		3. replace that string with a slice of itself specified by the arguments

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	w - "
};

int op_validate(operation* o)
{
	if(o->argsl >= 1 && o->args[0]->itype != ITYPE_I64)
		fail("w - first argument should be i64");
	if(o->argsl >= 2 && o->args[1]->itype != ITYPE_I64)
		fail("w - second argument should be i64");

	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int isselect = o->argsl == 3 && o->args[2]->l && strchr(o->args[2]->s, 'y');

	int newsl = (ls->s[0].l / 8) + 1;
	uint8_t* news;

	if(isselect)
	{
		news = alloca(newsl);
		memset(news, 0, newsl);
	}

	int x;
	for(x = 0; x < ls->s[0].l; x++)
	{
		int go = 1;
		if(ls->sel.l)
		{
			if(ls->sel.sl <= (x/8))
				break;

			go = (ls->sel.s[x/8] & (0x01 << (x%8)));
		}

		if(go)
		{
			int off = 0;
			int len = 0;

			if(o->argsl >= 1)
				off = o->args[0]->i64;
			if(o->argsl >= 2)
				len = o->args[1]->i64;

			if(off < 0)
				off = ls->s[0].s[x].l + off;
			if(len == 0)
				len = ls->s[0].s[x].l;

			if(ls->s[0].s[x].l >= (off + len))
			{
				// copy of the starting string
				int ssl = ls->s[0].s[x].l;
				char * ss = alloca(ssl + 1);
				memcpy(ss, ls->s[0].s[x].s, ssl);
				ss[ssl] = 0;

				// clear this string on the stack
				lstack_clear(ls, 0, x);

				fatal(lstack_write, ls, 0, x, ss + off, len);

				if(isselect)
				{
					news[x/8] |= (0x01 << (x%8));
				}
			}
		}
	}

	if(isselect)
		fatal(lstack_sel_write, ls, news, newsl);

	return 1;
}
