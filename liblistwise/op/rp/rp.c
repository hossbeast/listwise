#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <listwise/operator.h>

#include "control.h"

/*

rp operator - path canonicalization (with realpath)

NO ARGUMENTS

 use current selection, ELSE
 use entire top list

OPERATION

 1. rewrite the item as a canonicalized path

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	rp - "
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int l = ls->s[0].l;
	if(ls->sel.l)
		l = ls->sel.l;

	int x;
	for(x = 0; x < l; x++)
	{
		int k = x;
		if(ls->sel.l)
			k = ls->sel.s[x];

		char * ss = alloca(256);

		if(realpath(ls->s[0].s[k].s, ss))
		{
			fatal(lstack_write
				, ls
				, 0
				, k
				, ss
				, strlen(ss)
			);
		}
		else
		{
			printf("[%d][%s]\n", errno, strerror(errno));
		}
	}

	return 1;
}
