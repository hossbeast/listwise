#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <listwise/operator.h>

#include "control.h"

#include "parseint.h"

/*

s operator - search and replace

ARGUMENTS

  0  - regex
	1  - replacement string
	*2 - options string

OPERATION

	1. foreach item in selection, or, if no selection, in top list
	2. if regex matches, apply replacement

*/

static int op_validate(operation* o);
static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .type					= OPTYPE_GENERAL
	, .op_validate	= op_validate
	, .op_exec			= op_exec
	, .desc					= "	s - "
};

int op_validate(operation* o)
{
	if(o->argsl == 2)
		fatal(re_compile, o->args[0]->s, &o->args[0]->re, 0);
	else if(o->argsl == 3)
		fatal(re_compile, o->args[0]->s, &o->args[0]->re, o->args[2]->s);
	else
		fail("s -- argsl: %d", o->argsl);

	o->args[0]->itype = 1;

	// validate backreferences in the substitution string
	int x;
	for(x = 0; x < o->args[1]->refsl; x++)
	{
		fatal(parseint
			, o->args[1]->refs[x].s + 1	// 1 for the backslash
			, SCNd8
			, 0
			, 32
			, 0
			, 2
			, &o->args[1]->refs[x].bref
			, 0
		);

		if(o->args[1]->refs[x].bref > o->args[0]->re.c_caps)
		{
			fail("backref: %d, captures: %d", o->args[1]->refs[x].bref, o->args[0]->re.c_caps);
		}
	}

	return 1;
}

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	int l = ls->s[0].l;
	if(ls->sel.l)
		l = ls->sel.l;

	int isglobal = o->argsl == 3 && strchr(o->args[2]->s, 'g');

	int x;
	for(x = 0; x < l; x++)
	{
		int k = x;
		if(ls->sel.l)
			k = ls->sel.s[x];

		// ls->s[0].s[k].s - the string to check and modify
		fatal(re_exec, &o->args[0]->re, ls->s[0].s[k].s, ls->s[0].s[k].l, 0, ovec, ovec_len);

		if((*ovec)[0] > 0)
		{
			// copy of the starting string
			int ssl = ls->s[0].s[k].l;
			char * ss = alloca(ssl + 1);
			memcpy(ss, ls->s[0].s[k].s, ssl);
			ss[ssl] = 0;

			// offset to the end of the last match
			int loff = 0;

			// clear this string on the stack
			lstack_clear(ls, 0, k);

			// text in the subject string before the first match
			fatal(lstack_append
				, ls
				, 0
				, k
				, ss
				, (*ovec)[1]
			);

			do
			{
				// text in the subject string following the previous match, if any, and preceeding the current match
				if(loff)
				{
					fatal(lstack_append
						, ls
						, 0
						, k
						, ss + loff
						, (*ovec)[1] - loff
					);
				}

				// text in the replacement string before the first backreference
				if(o->args[1]->refsl)
				{
					fatal(lstack_append
						, ls
						, 0
						, k
						, o->args[1]->s
						, o->args[1]->refs[0].s - o->args[1]->s
					);
				}
				else
				{
					fatal(lstack_append
						, ls
						, 0
						, k
						, o->args[1]->s
						, o->args[1]->l
					);
				}

				// foreach backreference
				int i;
				for(i = 0; i < o->args[1]->refsl; i++)
				{
					// text in the replacement string between this and the previous backreference
					if(i)
					{
						fatal(lstack_append
							, ls
							, 0
							, k
							, o->args[1]->refs[i-1].e
							, o->args[1]->refs[i].s - o->args[1]->refs[i-1].e
						);
					}

					// text of the backreference itself, if the corresponding subcapture was populated
					if((*ovec)[0] > o->args[1]->refs[i].bref)
					{
						int a = (*ovec)[1 + (o->args[1]->refs[i].bref * 2) + 0];
						int b = (*ovec)[1 + (o->args[1]->refs[i].bref * 2) + 1];

						if(a >= 0 && b >= 0)
							fatal(lstack_append, ls, 0, k, ss + a, b - a);
					}
				}

				// text in the replacement string following the last backreference
				if(o->args[1]->refsl)
				{
					fatal(lstack_append
						, ls
						, 0
						, k
						, o->args[1]->ref_last->e
						, o->args[1]->l - (o->args[1]->ref_last->e - o->args[1]->s) 
					);
				}

				loff = (*ovec)[2];

				if(isglobal)
				{
					fatal(re_exec, &o->args[0]->re, ss, ssl, loff, ovec, ovec_len);
				}
			} while(isglobal && (*ovec)[0] > 0);

			// text in the subject string following the last match
			fatal(lstack_append
				, ls
				, 0
				, k
				, ss + loff
				, ssl - loff
			);
		}
	}

	return 1;
}
