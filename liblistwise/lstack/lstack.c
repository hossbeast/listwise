#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "listwise/internal.h"

#include "generator/generator.tab.h"

#include "xmem.h"
#include "xstring.h"
#include "macros.h"
#include "control.h"

//
// static
//

/// allocate
//
// SUMMARY
//  make an assertion on allocated dimensions of an lstack
//
// PARAMETERS
//  ls - lstack
//  x  - stack index
//  y  - string index
//  z  - character index
//
static int allocate(lstack * const restrict ls, int x, int y, int z)
{
	if(x >= 0)
	{
		if(ls->a <= x)
		{
			int ns = ls->a ?: 10;
			while(ns <= x)
				ns = ns * 2 + ns / 2;

			fatal(xrealloc, &ls->s, sizeof(*ls->s), ns, ls->a);
			ls->a = ns;
		}

		if(y >= 0)
		{
			if(ls->s[x].a <= y)
			{
				int ns = ls->s[x].a ?: 10;
				while(ns <= y)
					ns = ns * 2 + ns / 2;

				fatal(xrealloc	
					, &ls->s[x].s
					, sizeof(*ls->s[0].s)
					, ns
					, ls->s[x].a
				);

				// list of tmp space
				fatal(xrealloc
					, &ls->s[x].t
					, sizeof(*ls->s[0].t)
					, ns
					, ls->s[x].a
				);

				ls->s[x].a = ns;
			}

			if(z >= 0)
			{
				if(ls->s[x].s[y].a <= z)
				{
					int ns = ls->s[x].s[y].a ?: 10;
					while(ns <= z)
						ns = ns * 2 + ns / 2;

					fatal(xrealloc
						, &ls->s[x].s[y].s
						, sizeof(*ls->s[0].s[0].s)
						, ns
						, ls->s[x].s[y].a
					);

					ls->s[x].s[y].a = ns;
				}
			}
		}
	}

	finally : coda;
}

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

	finally : coda;
}

static int writestack_alt(lstack* const restrict ls, int x, int y, const void* const restrict s, int l, uint8_t type)
{
	if(type)
	{
		// ensure stack has enough lists, list has enough strings, string has enough bytes
		fatal(allocate, ls, x, y, sizeof(s) - 1);

		// copy the pointer, set the type
		memcpy(ls->s[x].s[y].s, (void*)&s, sizeof(s));
		ls->s[x].s[y].type = type;
		ls->s[x].s[y].l = 0;
	}
	else
	{
		fatal(allocate, ls, x, y, l);

		// write and cap the string
		memcpy(ls->s[x].s[y].s, s, l);
		ls->s[x].s[y].s[l] = 0;
		ls->s[x].s[y].l = l;
		ls->s[x].s[y].type = 0;
	}

	// dirty the temp space for this entry
	ls->s[x].t[y].w = 0;

	finally : coda;
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

	finally : coda;
}

static int vwritestack(lstack* const restrict ls, int x, int y, const char* const restrict fmt, va_list va)
{
	va_list va2;
	va_copy(va2, va);

	int l = vsnprintf(0, 0, fmt, va);
	va_end(va);

	fatal(ensure, ls, x, y, l);

	vsprintf(ls->s[x].s[y].s, fmt, va2);
	va_end(va2);

	ls->s[x].s[y].l = l;
	ls->s[x].s[y].type = 0;

	// dirty the temp space for this entry
	ls->s[x].t[y].w = 0;

	finally : coda;
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

	lstack_sanity(*ls);

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

			lstack_sanity(*ls);

			dprintf(listwise_err_fd, "\n");
			dprintf(listwise_err_fd, " >> %s", g->ops[x]->op->s);

			int y;
			for(y = 0; y < g->ops[x]->argsl; y++)
				dprintf(listwise_err_fd, "/%s", g->ops[x]->args[y]->s);

			dprintf(listwise_err_fd, "\n");
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

	lstack_sanity(*ls);

finally:
	free(ovec);
coda;
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

int API lstack_create(lstack ** const restrict ls)
{
	return xmalloc(ls, sizeof**ls);
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
			for(y = 0; y < ls->s[x].a; y++)
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
			dprintf(listwise_err_fd, "\n");

		if(ls->s[x].l == 0)
		{
			dprintf(listwise_err_fd, "[%4d     ] -- empty \n", x);
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

			dprintf(listwise_err_fd, "[%4d,%4d] %s%s "
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

				dprintf(listwise_err_fd, "[%hhu]%p/%p (%.*s)", ls->s[x].s[y].type, *(void**)ls->s[x].s[y].s, ls->s[x].s[y].s, l, s);
			}
			else
			{
				dprintf(listwise_err_fd, "'%.*s'", ls->s[x].s[y].l, ls->s[x].s[y].s);
			}
			dprintf(listwise_err_fd, "\n");
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

finally:
coda;
}

int API lstack_appendf(lstack* const restrict ls, int x, int y, const char* const restrict s, ...)
{
	return 0;
}

int API lstack_write_alt(lstack* const restrict ls, int x, int y, const char* const restrict s, int l)
{
	return writestack_alt(ls, x, y, s, l, 0);
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

int API lstack_obj_write_alt(lstack* const restrict ls, int x, int y, const void* const restrict o, uint8_t type)
{
	return writestack_alt(ls, x, y, o, 0, type);
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

// shift removes the first list from the stack
int API lstack_shift(lstack* const restrict ls)
{
	if(ls->l)
	{
		typeof(ls->s[0]) T = ls->s[0];

		memmove(
			  &ls->s[0]
			, &ls->s[1]
			, (ls->l - 1) * sizeof(ls->s[0])
		);

		ls->s[--ls->l] = T;
	}
}

// pop removes the last list from the stack
int API lstack_pop(lstack* const restrict ls)
{
	if(ls->l)
		ls->l--;
}

// unshift allocates a new list at index 0
int API lstack_unshift(lstack* const restrict ls)
{
	// allocate new list, if necessary
	if(ls->l == ls->a)
	{
		fatal(xrealloc, &ls->s, sizeof(ls->s[0]), ls->l + 1, ls->a);
		ls->a++;
	}

	ls->l++;

	// swap new list into position zero
	typeof(ls->s[0]) T = ls->s[ls->l - 1];

	// copy list pointers down
	memmove(
			&ls->s[1]
		, &ls->s[0]
		, (ls->l - 1) * sizeof(ls->s[0])
	);

	ls->s[0] = T;

	finally : coda;
}

// push allocates a new list at the end
int API lstack_push(lstack* const restrict ls)
{
	// allocate new list, if necessary
	if(ls->l == ls->a)
	{
		fatal(xrealloc, &ls->s, sizeof(ls->s[0]), ls->l + 1, ls->a);
		ls->a++;
	}

	// reset
	ls->s[ls->l++].l = 0;

	finally : coda;
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

	finally : coda;
}

int API lstack_allocate(lstack * const restrict ls, int x, int y, int z)
{
	return allocate(ls, x, y, z);
}

int API lstack_ensure(lstack * const restrict ls, int x, int y, int z)
{
	return ensure(ls, x, y, z);
}

int API lstack_move(lstack * const restrict ls, int ax, int ay, int bx, int by)
{
	// copy of ax:ay, which is about to be overwritten
	typeof(ls->s[0].s[0]) Ts = ls->s[ax].s[ay];
	typeof(ls->s[0].t[0]) Tt = ls->s[ax].t[ay];

	// copy
	ls->s[ax].s[ay] = ls->s[bx].s[by];
	ls->s[ax].t[ay] = ls->s[bx].t[by];

	// overwrite bx:by, creating a duplicate entry
	memmove(
	    &ls->s[bx].s[by]
		, &ls->s[bx].s[by+1]
		, (ls->s[bx].l - by - 1) * sizeof(ls->s[0].s[0])
	);

	memmove(
		  &ls->s[bx].t[by]
		, &ls->s[bx].t[by+1]
		, (ls->s[bx].l - by - 1) * sizeof(ls->s[0].t[0])
	);

	// overwrite the duplicate entry with the original dest
	ls->s[bx].s[ls->s[bx].l - 1] = Ts;
	ls->s[bx].t[ls->s[bx].l - 1] = Tt;

	// adjust the length
	ls->s[bx].l--;

	return 1;
}

int API lstack_delete(lstack * const restrict ls, int x, int y)
{
	// copy of this entry
	typeof(ls->s[0].s[0]) Ts = ls->s[x].s[y];
	typeof(ls->s[0].t[0]) Tt = ls->s[x].t[y];

	// overwrite this entry
	memmove(
			&ls->s[x].s[y]
		, &ls->s[x].s[y+1]
		, (ls->s[x].l - y - 1) * sizeof(ls->s[0].s[0])
	);

	memmove(
			&ls->s[x].t[y]
		, &ls->s[x].t[y+1]
		, (ls->s[x].l - x - 1) * sizeof(ls->s[0].t[0])
	);

	// overwrite the duplicated entry with the current entry
	ls->s[x].s[ls->s[x].l - 1] = Ts;
	ls->s[x].s[ls->s[x].l - 1] = Ts;

	ls->s[x].l--;

	return 1;
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
				if(xrealloc(
					  &ls->s[x].t[y].s
					, sizeof(ls->s[x].t[y].s[0])
					, rl + 1
					, ls->s[x].t[y].a) == 0)
				{
					return 0;
				}

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

void lstack_sanity(lstack * const restrict ls)
{
	int R = 0;

#define err(...) dprintf(listwise_err_fd, __VA_ARGS__); R++

	int ax;
	int ay;
	int bx;
	int by;
	for(ax = 0; ax < ls->l; ax++)
	{
		for(ay = 0; ay < ls->s[ax].l; ay++)
		{
			for(bx = 0; bx < ls->l; bx++)
			{
				if(ax != bx)
				{
					if(ls->s[ax].s == ls->s[bx].s)
					{
						err("duplicate lists %p a=[%d] b=[%d]\n", ls->s[ax].s, ax, bx);
					}
				}

				for(by = 0; by < ls->s[bx].l; by++)
				{
					if(ax != bx || ay != by)
					{
						if(ls->s[ax].s[ay].s == ls->s[bx].s[by].s)
						{
							err("duplicate strings %p a=[%d,%d] b=[%d,%d]\n", ls->s[ax].s[ay].s, ax, ay, bx, by);
						}
					}
				}
			}
		}
	}

	if(R)
		exit(1);
}
