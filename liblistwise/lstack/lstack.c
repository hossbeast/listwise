#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "listwise/internal.h"

#include "generator.tab.h"

#include "xmem.h"
#include "xstring.h"
#include "macros.h"
#include "control.h"

//
// static
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
				// list of strings
				fatal(xrealloc	
					, &ls->s[x].s
					, sizeof(ls->s[x].s[0])
					, y + 1
					, ls->s[x].a
				);

				// list of tmp space
				fatal(xrealloc
					, &ls->s[x].t
					, sizeof(ls->s[x].t[0])
					, y + 1
					, ls->s[x].a
				);

				ls->s[x].a = y + 1;
			}

			if(ls->s[x].l <= y)
				ls->s[x].l = y + 1;

			if(z > 0)
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

static int writestack(lstack* const restrict ls, int x, int y, const void* const restrict s, int l, uint8_t type)
{
	if(type)
	{
		// ensure stack has enough lists, list has enough strings, string has enough bytes
		fatal(ensure, ls, x, y, sizeof(s));

		// copy the pointer, set the type
		memcpy(ls->s[x].s[y].s, (void*)&s, sizeof(s));
		ls->s[x].s[y].type = type;
		ls->s[x].s[y].l = 0;
	}
	else
	{
		fatal(ensure, ls, x, y, l);

		// write and cap the string
		memcpy(ls->s[x].s[y].s, s, l);
		ls->s[x].s[y].s[l] = 0;
		ls->s[x].s[y].l = l;
		ls->s[x].s[y].type = 0;
	}

	// dirty the temp space for this entry
	ls->s[x].t[y].w = 0;

	return 1;
}

static int vwritestack(lstack* const restrict ls, int x, int y, const char* const restrict fmt, va_list va)
{
	va_list va2;
	va_copy(va2, va);

	int l = vsnprintf(0, 0, fmt, va);
	va_end(va);

	fatal(ensure, ls, x, y, l + 1);

	vsprintf(ls->s[x].s[y].s, fmt, va2);
	va_end(va2);

	ls->s[x].s[y].l = l;
	ls->s[x].s[y].type = 0;
	ls->s[x].t[y].w = 0;

	return 1;
}

static int exec_internal(generator* g, char** init, int* initls, int initl, lstack** ls, int dump)
{
	// ovec workspace
	int* ovec = 0;
	int ovec_len = 0;

	// empty operation for implicit y operator execution
	operator* yop = op_lookup("y", 1);
	operator* oop = op_lookup("o", 1);

	struct operation yoper = { .op = yop };

	// list stack allocation
	if(!*ls)
		fatal(xmalloc, ls, sizeof(*ls[0]));

	// write init elements to top of list stack
	int x;
	for(x = 0; x < initl; x++)
	{
		fatal(writestack, *ls, 0, x, init[x], initls[x], 0);
	}

	// write initial generator args at top of list stack
	for(x = 0; x < g->argsl; x++)
	{
		fatal(writestack, *ls, 0, x + initl, g->args[x]->s, g->args[x]->l, 0);
	}

	// the initial state of the selection is all
	fatal(lstack_sel_all, *ls);

	if(dump)
		lstack_dump(*ls);

	// execute all operations
	int isor = 0;
	for(x = 0; x < g->opsl; x++)
	{
		isor = 0;
		if(g->ops[x]->op == oop)
		{
			isor = 1;
			if(++x == g->opsl)
				break;
		}

		if(!isor)
		{
			if(x && (g->ops[x-1]->op->optype & LWOP_SELECTION_WRITE))
				fatal(yoper.op->op_exec, &yoper, *ls, &ovec, &ovec_len);

			if(g->ops[x]->op != yop)
				fatal(lstack_last_clear, *ls);
		}

		if(dump)
		{
			if(x)
				lstack_dump(*ls);

			printf("\n");
			printf(" >> %s", g->ops[x]->op->s);

			int y;
			for(y = 0; y < g->ops[x]->argsl; y++)
				printf("/%s", g->ops[x]->args[y]->s);

			printf("\n");
		}

		// execute the operator
		fatal(g->ops[x]->op->op_exec, g->ops[x], *ls, &ovec, &ovec_len);
	}

	if(x && (g->ops[x-1]->op->optype & LWOP_SELECTION_WRITE))
		fatal(yoper.op->op_exec, &yoper, *ls, &ovec, &ovec_len);

	fatal(lstack_last_clear, *ls);

	// clear string props set with the fx operator
	for(x = 0; x < object_registry.l; x++)
		xfree(&object_registry.e[x]->string_property);

	if(dump)
		lstack_dump(*ls);

	free(ovec);
	return 1;
}

//
// API
//

int API lstack_exec_internal(generator* g, char** init, int* initls, int initl, lstack** ls, int dump)
{
	return exec_internal(g, init, initls, initl, ls, dump);
}

int API lstack_exec(generator* g, char** init, int* initls, int initl, lstack** ls)
{
	return exec_internal(g, init, initls, initl, ls, 0);
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
			{
				free(ls->s[x].s[y].s);
				free(ls->s[x].t[y].s);
			}

			free(ls->s[x].s);
			free(ls->s[x].t);
		}

		free(ls->s);
		free(ls->sel.s);
		free(ls->last.s);
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
			{
				ls->s[x].s[y].l = 0;
				ls->s[x].s[y].type = 0;
				ls->s[x].t[y].w = 0;
			}

			ls->s[x].l = 0;
		}

		ls->l = 0;
		ls->sel.all = 1;
	}
}

void API lstack_dump(lstack* ls)
{
	int x;
	int y;
	for(x = ls->l - 1; x >= 0; x--)
	{
		if(x != ls->l - 1)
			printf("\n");

		if(ls->s[x].l == 0)
		{
			printf("[%4d     ] -- empty \n", x);
		}

		for(y = 0; y < ls->s[x].l; y++)
		{
			int sel = 0;
			int last = 0;
			if(x == 0)
			{
				sel = 1;
				if(!ls->sel.all)
				{
					if(ls->sel.sl <= (y/8))
						sel = 0;
					else
					{
						if((ls->sel.s[y/8] & (0x01 << (y%8))) == 0)
							sel = 0;
					}
				}

				if(ls->last.sl > (y/8))
				{
					if(ls->last.s[y/8] & (0x01 << (y%8)))
						last = 1;
				}
			}

			printf("[%4d,%4d] %s%s "
				, x
				, y
				, sel ? ">" : " "
				, last ? ">" : " "
			);

			if(ls->s[x].s[y].type)
			{
				char * s;
				int l;
				lstack_string(ls, x, y, &s, &l);

				printf("[%hhu]%p (%.*s)", ls->s[x].s[y].type, *(void**)ls->s[x].s[y].s, l, s);
			}
			else
			{
				printf("'%.*s'", ls->s[x].s[y].l, ls->s[x].s[y].s);
			}
			printf("\n");
		}
	}
}

void API lstack_clear(const lstack * const restrict ls, int x, int y)
{
	ls->s[x].s[y].l = 0;
	ls->s[x].s[y].type = 0;
	ls->s[x].t[y].w = 0;
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
	ls->s[x].s[y].type = 0;

	ls->s[x].t[y].w = 0;

	return 1;
}

int API lstack_appendf(lstack* const restrict ls, int x, int y, const char* const restrict s, ...)
{
	return 0;
}

int API lstack_write(lstack* const restrict ls, int x, int y, const char* const restrict s, int l)
{
	return writestack(ls, x, y, s, l, 0);
}

int API lstack_writef(lstack* const restrict ls, int x, int y, const char* const restrict fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	return vwritestack(ls, x, y, fmt, va);
}

int API lstack_obj_write(lstack* const restrict ls, int x, int y, const void* const restrict o, uint8_t type)
{
	return writestack(ls, x, y, o, 0, type);
}

int API lstack_add(lstack* const restrict ls, const char* const restrict s, int l)
{
	return writestack(ls, 0, ls->l ? ls->s[0].l : 0, s, l, 0);
}

int API lstack_addf(lstack* const restrict ls, const char* const restrict fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	return vwritestack(ls, 0, ls->l ? ls->s[0].l : 0, fmt, va);
}

int API lstack_obj_add(lstack* const restrict ls, const void* const restrict o, uint8_t type)
{
	return writestack(ls, 0, ls->l ? ls->s[0].l : 0, o, 0, type);
}

int API lstack_shift(lstack* const restrict ls)
{
	if(ls->l)
		ls->l--;
}

int API lstack_unshift(lstack* const restrict ls)
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
	fatal(xmalloc, &ls->s[0].t, sizeof(ls->s[0].t[0]));
	ls->s[0].l = 0;
	ls->s[0].a = 0;

	return 1;
}

int API lstack_pop(lstack* const restrict ls)
{
	if(ls->l)
	{
		typeof(ls->s[0]) T = ls->s[0];

		memmove(
			  &ls->s[0]
			, &ls->s[1]
			, (ls->l - 1) * sizeof(ls->s[0])
		);

		ls->l--;

		ls->s[ls->l] = T;
	}
}

int API lstack_push(lstack* const restrict ls)
{
	// allocate new spot
	fatal(ensure, ls, ls->l, -1, -1);

	// allocate new list at the end
	fatal(xmalloc, &ls->s[ls->l - 1].s, sizeof(ls->s[0].s[0]));
	ls->s[ls->l - 1].l = 0;
	ls->s[ls->l - 1].a = 0;
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

int API lstack_merge(lstack* const restrict ls, int to, int from)
{
	fatal(ensure, ls, to, -1, -1);
	int tox = ls->s[to].l;
	fatal(ensure, ls, to, ls->s[to].l + ls->s[from].l - 1, -1);

	memcpy(
		  ls->s[to].s + tox
		, ls->s[from].s
		, ls->s[from].l * sizeof(ls->s[0].s[0])
	);

	ls->s[from].a = 0;
	ls->s[from].l = 0;

	while(from < ls->l && ls->s[from].l == 0)
		from++;
	from--;

	while(from >= 0 && ls->s[from].l == 0)
		from--;
	from++;

	ls->l = from;

	return 1;
}

int API lstack_ensure(lstack * const restrict ls, int x, int y, int z)
{
	return ensure(ls, x, y, z);
}

int API lstack_move(lstack * const restrict ls, int ax, int ay, int bx, int by)
{
	fatal(lstack_ensure, ls, ax, ay, 0);

	// copy
	ls->s[ax].s[ay] = ls->s[bx].s[by];

	// delete
	memmove(
	    &ls->s[bx].s[by]
		, &ls->s[bx].s[by+1]
		, (ls->s[bx].l - by - 1) * sizeof(ls->s[0].s[0])
	);

	ls->s[bx].l--;
	ls->s[bx].a--;

	ls->s[bx].t[ay].w = 0;
	ls->s[bx].t[by].w = 0;
}

int API lstack_string(lstack* const restrict ls, int x, int y, char ** r, int * rl)
{
	if(ls->s[x].s[y].type)
	{
		listwise_object* o = idx_lookup_val(object_registry.by_type, &ls->s[x].s[y].type, 0);

		if(o)
		{
			o->string(*(void**)ls->s[x].s[y].s, o->string_property, r, rl);

			return 1;
		}

		return 0;
	}

	*r  = ls->s[x].s[y].s;
	*rl = ls->s[x].s[y].l;

	return 1;
}

typedef char* charstar;
charstar API lstack_getstring(lstack* const restrict ls, int x, int y)
{
	if(ls->s[x].t[y].w == 0)
	{
		char * r = 0;
		int    rl = 0;

		if(lstack_string(ls, x, y, &r, &rl))
		{
			if(ls->s[x].t[y].a <= rl)
			{
				fatal(xrealloc
					, &ls->s[x].t[y].s
					, sizeof(ls->s[x].t[y].s[0])
					, rl + 1
					, ls->s[x].t[y].a
				);

				ls->s[x].t[y].a = rl + 1;
			}

			memcpy(ls->s[x].t[y].s, r, rl);
			ls->s[x].t[y].s[rl] = 0;
			ls->s[x].t[y].l = rl;
			ls->s[x].t[y].w = 1;
		}
		else
		{
			return 0;
		}
	}

	return ls->s[x].t[y].s;
}
