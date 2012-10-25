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
	int newsl = ls->sel.sl;
	uint8_t * news = alloca(ls->sel.sl * sizeof(news[0]));
	memset(news, 0, newsl * sizeof(ls->sel.s[0]));

	int x;
	for(x = 0; x < newsl * 8; x++)
	{
		if(x == ls->s[0].l)
			break;

		if((ls->sel.s[x/8] & (0x01 << (x%8))) == 0)
			news[x/8] |= (0x01 << (x%8));
	}

	fatal(lstack_sel_write, ls, news, newsl);

	return 1;
}
