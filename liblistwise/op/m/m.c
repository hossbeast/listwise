#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>

#include "control.h"

/*

m operator - match (select by regex)

ARGUMENTS

  0  - regex
	1* - options string

OPERATION

	1. foreach item in selection, or, if no selection, in top list
	2. select that item if regex matches, subject to options

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	m - "
};

int op_validate(operation* o)
{
	if(o->argsl == 1 || o->argsl == 2)
	{
		if(o->args[0]->l == 0)
			fail("m -- empty first argument");

		if(o->argsl == 1 || o->args[1]->l == 0)
			fatal(re_compile, o->args[0]->s, &o->args[0]->re, 0);
		else if(o->argsl == 2)
			fatal(re_compile, o->args[0]->s, &o->args[0]->re, o->args[1]->s);
	}
	else
		fail("m -- arguments : %d", o->argsl);

	o->args[0]->itype = 1;

	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	// enough room to select all elements
	int newsl = (ls->s[0].l / 8) + 1;
	uint8_t * news = alloca(newsl);
	memset(news, 0, newsl);

	int x;
	for(x = 0; x < ls->s[0].l; x++)
	{
		int go = 1;
		if(ls->sel.l)
		{
			if(ls->sel.sl <= (x/8))	// could not be selected
				break;

			go = (ls->sel.s[x/8] & (0x01 << (x%8)));	// whether it is selected
		}

		if(go)
		{
			fatal(re_exec, &o->args[0]->re, ls->s[0].s[x].s, ls->s[0].s[x].l, 0, ovec, ovec_len);

			if((*ovec)[0] > 0)
				news[x/8] |= (0x01 << (x%8));
		}
	}

	lstack_sel_write(ls, news, newsl);

	return 1;
}
