#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "listwise/internal.h"

#include "generator.tab.h"

#include "xmem.h"
#include "xstring.h"
#include "macros.h"

//
// static
//

/// ensure
//
// ensure stack/list allocation up to the specified dimensions
//
// parameters
//   x - zero based list index       ( or -1 to skip stack allocation)
//   y - zero based string index     ( or -1 to skip list allocation)
//   z - zero based character index  ( or -1 to skip string allocation)
//
// updates stack and list length to be at least the dimension specified
//  ** this is NOT done for string length **
//
static int ensure(lstack * const restrict ls, int x, int y, int z)
{
	if(x >= 0)
	{
		// ensure stack has enough lists
		if(ls->a <= x)
		{
			fatal(xrealloc
				, &ls->s
				, sizeof(ls->s[0])
				, x + 1
				, ls->a
			);

			ls->a = x + 1;
		}

		if(ls->l <= x)
			ls->l = x + 1;

		if(y >= 0)
		{
			// ensure list has enough strings
			if(ls->s[x].a <= y)
			{
				fatal(xrealloc
					, &ls->s[x].s
					, sizeof(ls->s[x].s[0])
					, y + 1
					, ls->s[x].a
				);

				ls->s[x].a = y + 1;
			}

			if(ls->s[x].l <= y)
				ls->s[x].l = y + 1;

			if(z >= 0)
			{
				// ensure string has enough space
				if(ls->s[x].s[y].a <= z)
				{
					fatal(xrealloc
						, &ls->s[x].s[y].s
						, sizeof(ls->s[x].s[y].s[0])
						, z + 1
						, ls->s[x].s[y].a
					);

					ls->s[x].s[y].a = z + 1;
				}
			}
		}
	}

	return 1;
}

static int writestack(lstack* const restrict ls, int x, int y, const char* const restrict s, int l)
{
	// ensure stack has enough lists, list has enough strings, string has enough bytes
	fatal(ensure, ls, x, y, l + 1);

	// write and cap the string
	memcpy(ls->s[x].s[y].s, s, l);
	ls->s[x].s[y].s[l] = 0;
	ls->s[x].s[y].l = l;

	return 1;
}

//
// API
//

int API lstack_exec(generator* g, char** init, int* initls, int initl, lstack** ls)
{
	// ovec workspace
	int* ovec = 0;
	int ovec_len = 0;

	// list stack allocation
	if(!*ls)
		fatal(xmalloc, ls, sizeof(*ls[0]));

	// write init elements to top of list stack
	int x;
	for(x = 0; x < initl; x++)
	{
		fatal(writestack, *ls, 0, x, init[x], initls[x]);
	}

	// write initial generator args at top of list stack
	for(x = 0; x < g->argsl; x++)
	{
		fatal(writestack, *ls, 0, x + initl, g->args[x]->s, g->args[x]->l);
	}

	// execute all operators
	for(x = 0; x < g->opsl; x++)
	{
		fatal(g->ops[x]->op->op_exec, g->ops[x], *ls, &ovec, &ovec_len);
	}

	free(ovec);
	return 1;
}

void API lstack_free(lstack* ls)
{
	if(ls)
	{
		int x;
		int y;
		for(x = 0; x < ls->a; x++)
		{
			for(y = 0; y < ls->s[x].a; y++)
				free(ls->s[x].s[y].s);

			free(ls->s[x].s);
		}

		free(ls->s);
	}

	free(ls);
}

void API lstack_xfree(lstack** ls)
{
	lstack_free(*ls);
	*ls = 0;
}

void API lstack_reset(lstack* ls)
{
	int x;
	int y;

	if(ls)
	{
		for(x = 0; x < ls->l; x++)
		{
			for(y = 0; y < ls->s[x].l; y++)
				ls->s[x].s[y].l = 0;

			ls->s[x].l = 0;
		}

		ls->l = 0;
	}
}

void API lstack_dump(lstack* ls)
{
	int x;
	int y;
	int z = 0;
	for(x = 0; x < ls->l; x++)
	{
		if(x)
			printf("\n");

		for(y = 0; y < ls->s[x].l; y++)
		{
			int sel = 0;
			if(x == 0)
			{
				if(z < ls->sel.l && y == ls->sel.s[z])
					sel++;
			}

			printf("[%4d,%4d]%s'%.*s'\n"
				, x
				, y
				, sel ? " >< " : "    "
				, ls->s[x].s[y].l
				, ls->s[x].s[y].s
			);

			if(sel)
				z++;
		}
	}
}

void API lstack_clear(const lstack * const restrict ls, int x, int y)
{
	ls->s[x].s[y].l = 0;
}

int API lstack_append(lstack * const restrict ls, int x, int y, const char* const restrict s, int l)
{
	// ensure stack has enough lists, stack has enough strings, string has enough bytes
	fatal(ensure, ls, x, y, -1);
	fatal(ensure, ls, x, y, ls->s[x].s[y].l + l + 1);

	// append and cap the string
	memcpy(ls->s[x].s[y].s + ls->s[x].s[y].l, s, l);
	ls->s[x].s[y].s[ls->s[x].s[y].l + l] = 0;
	ls->s[x].s[y].l += l;

	return 1;
}

int API lstack_appendf(lstack* const restrict ls, int x, int y, const char* const restrict s, ...)
{
	return 0;
}

int API lstack_write(lstack* const restrict ls, int x, int y, const char* const restrict s, int l)
{
	return writestack(ls, x, y, s, l);
}

int API lstack_writef(lstack* const restrict ls, int x, int y, const char* const restrict fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	va_list va2;
	va_copy(va2, va);

	int l = vsnprintf(0, 0, fmt, va);
	va_end(va);

	fatal(ensure, ls, x, y, l + 1);

	vsprintf(ls->s[x].s[y].s, fmt, va2);
	va_end(va2);

	ls->s[x].s[y].l = l;

	return 1;
}

int API lstack_push(lstack* const restrict ls)
{
	// ensure stack has enough lists
	fatal(ensure, ls, ls->l, -1, -1);

	// copy list pointers down
	memmove(
			&ls->s[1]
		, &ls->s[0]
		, (ls->l - 1) * sizeof(ls->s[0])
	);

	// allocate new list at index 0
	fatal(xmalloc, &ls->s[0].s, sizeof(ls->s[0].s[0]));
	ls->s[0].l = 0;
	ls->s[0].a = 0;

	return 1;
}

int API lstack_cycle(lstack* const restrict ls)
{
	return 0;
}

int API lstack_recycle(lstack* const restrict ls)
{
	return 0;
}

int API lstack_xchg(lstack* const restrict ls)
{
	return 0;
}
