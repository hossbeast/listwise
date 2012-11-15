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
	  .optype					= LWOP_SELECTION_READ | LWOP_SELECTION_RESET | LWOP_ARGS_CANHAVE | LWOP_OPERATION_PUSHBEFORE | LWOP_OPERATION_FILESYSTEM
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "create new list from directory listing(s)"
};

int op_validate(operation* o)
{
	return 1;
}

static int listing(lstack* ls, char * s, l)
{
	DIR * dd = 0;

	char tmp[l + 1];
	memcpy(tmp, s, l);
	tmp[l] = 0;

	if((dd = opendir(tmp)))
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
					if(strcmp(entp->d_name, ".") && strcmp(entp->d_name, ".."))
					{
						fatal(lstack_addf, ls, "%.*s/%s", l, s, entp->d_name);
						fatal(listing, ls, ls->s[0].s[ls->s[0].l - 1].s, ls->s[0].s[ls->s[0].l - 1].l);
					}
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
	else if(errno != ENOTDIR)
	{
		dprintf(listwise_err_fd, "opendir('%.*s')=[%d][%s]\n", l, s, errno, strerror(errno));
	}

	closedir(dd);
	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int x;
	fatal(lstack_unshift, ls);

	if(o->argsl)
	{
		for(x = 0; x < o->argsl; x++)
			fatal(listing, ls, o->args[x]->s);
	}
	else
	{
		for(x = 0; x < ls->s[1].l; x++)
		{
			int go = 1;
			if(!ls->sel.all)
			{
				if(ls->sel.sl <= (x/8))
					break;

				go = (ls->sel.s[x/8] & (0x01 << (x%8)));
			}

			if(go)
			{
				char * s;
				int l;
				lstack_string(ls, 1, x, &s, &l);

				fatal(listing, ls, s, l);
			}
		}
	}

	// if anything was selected, its now used up
	fatal(lstack_sel_all, ls);

	return 1;
}
