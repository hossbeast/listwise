#include <stdlib.h>

#include "listwise/internal.h"

#include "xmem.h"

int API lstack_sel_clear(lstack* const restrict ls)
{
	ls->sel.l = 0;

	return 1;
}

int API lstack_sel_add(lstack* const restrict ls, int y)
{
	return 0;
}

int API lstack_sel_write(lstack* const restrict ls, int y)
{
	if(ls->sel.l == ls->sel.a)
	{
		int ns = 8;
		if(ls->sel.a)
			ns = (ls->sel.a * 2) + (ls->sel.a / 2);
	
		fatal(xrealloc, &ls->sel.s, sizeof(ls->sel.s[0]), ns, ls->sel.a);
		ls->sel.a = ns;	
	}

	ls->sel.s[ls->sel.l++] = y;

	return 1;
}
