#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <listwise/operator.h>

#include "control.h"

#include "parseint.h"

/*

-f operator - select entries whose stringvalue is a path referencing a regular file

NO ARGUMENTS

 use current selection, ELSE
 use entire top list

OPERATION

 1. select entries referencing a regular file

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	-f - "
};

int op_validate(operation* o)
{
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
			struct stat st;
			if(stat(ls->s[0].s[x].s, &st) == 0)
			{
				if(S_ISREG(st.st_mode))
					news[x/8] |= (0x01 << (x%8));
			}
			else
			{
				dprintf(listwise_err_fd, "stat('%s')=[%d][%s]\n", ls->s[0].s[x].s, errno, strerror(errno));
			}
		}
	}

	fatal(lstack_sel_write, ls, news, newsl);

	return 1;
}
