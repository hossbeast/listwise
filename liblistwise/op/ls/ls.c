#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <listwise/operator.h>

#include "control.h"

#include "parseint.h"

/*

ls operator - directory listing

ARGUMENTS

 has arguments - use each
 no arguments  - use current selection
 no selection  - use entire top list

OPERATION

 1. push an empty list onto the stack

 2. for each argument
      argument is the path to a directory
      for each item in the the directory listing
        append path to the item (relative to the argument given)

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	ls- "
};

int op_validate(operation* o)
{
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int l = 0;

	if(o->argsl)
		l = o->argsl;
	else if(ls->sel.l)
		l = ls->sel.l;
	else
		l = ls->s[0].l;

	if(l)
	{
		fatal(lstack_push, ls);

		int x;
		int y = 0;
		for(x = 0; x < l; x++)
		{
			char * s = 0;

			if(o->argsl)
				s = o->args[x]->s;
			else if(ls->sel.l)
				s = ls->s[1].s[ls->sel.s[x]].s;
			else
				s = ls->s[1].s[x].s;

			DIR * dd = 0;

			if((dd = opendir(s)))
			{
				struct dirent ent;
				struct dirent * entp = 0;
				int r = 0;
				while(1)
				{
					if((r = readdir_r(dd, &ent, &entp)) == 0)
					{
						if(entp)
						{
							fatal(lstack_writef, ls, 0, y++, "%s/%s", s, entp->d_name);
						}
						else
						{
							break;
						}
					}
					else
					{
						fail("readdir('%s')=[%d][%s]", s, r, strerror(r));
					}
				}
			}
			else
			{
				fail("opendir('%s')=[%d][%s]", s, errno, strerror(errno));
			}

			closedir(dd);
		}
	}

	// if anything was selected, its now used up
	fatal(lstack_sel_clear, ls);

	return 1;
}
