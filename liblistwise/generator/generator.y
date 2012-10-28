%code top {
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdint.h>
	#include <string.h>

	#include "generator.def.h"

	int  generator_yylex(void* yylvalp, void* scanner);
	void generator_yyerror(void* scanner, parse_param* pp, char const* err);
}

%define api.pure
%token-table
%error-verbose
%name-prefix="generator_yy"
%parse-param { void* scanner }
%parse-param { parse_param* parm }
%lex-param { void* scanner }
%expect 5

%union {
	struct								// string segment
	{
		char*				s;
		int					l;
	}							str;

	struct
	{
		char *			s;
		int					l;
		operator *	v;
	}							op;

	struct
	{
		char *			s;
		int					l;
		int					v;
	}							bref;

	struct
	{
		char *			s;
		int					l;
		int64_t			v;
	}							i64;

	struct items
	{
		struct items * next;
		char t;											/* 1=op, 2=arg */

		union
		{
			operator * 	op;						/* operators */
			arg *				arg;					/* argument */
		};
	}						*	items;
}

%token	<op>			'/'

%token	<str>			STR
%token	<bref>		BREF
%token  <i64>		  I64
%token	<op>			OP

%type		<items>		items
%type		<items>		string

/* sugar */
%token END 0 "end of file"

%%

generator
	: items
	{
		// count operations and initial arguments
		parm->g->opsl = 0;
		parm->g->argsl = 0;

		struct items * i = $1;
		while(i)
		{
			if(i->t == 1)
				parm->g->opsl++;
			else if(parm->g->opsl == 0)
				parm->g->argsl++;

			i = i->next;
		}

		// allocate operations and initial arguments
		parm->g->args = calloc(parm->g->argsl, sizeof(parm->g->args[0]));
		parm->g->ops  = calloc(parm->g->opsl, sizeof(parm->g->ops[0]));

		i = $1;
		int x = -1;
		int j = 0;
		while(i)
		{
			if(i->t == 1)
			{
				x++;
				parm->g->ops[x] = calloc(1, sizeof(*parm->g->ops[0]));
				parm->g->ops[x]->op = i->op;

				// count args
				struct items * a = i->next;
				int y = 0;
				while(a && a->t == 2)
				{
					a = a->next;
					y++;
				}

				parm->g->ops[x]->args = calloc(y, sizeof(*parm->g->ops[0]->args));
			}
			else if(x == -1)
			{
				parm->g->args[j++] = i->arg;
			}
			else
			{
				parm->g->ops[x]->args[parm->g->ops[x]->argsl++] = i->arg;
			}

			struct items * tmp = i;
			i = i->next;
			free(tmp);
		}
	}
	;

items
	: items '/' items
	{
		$$ = $1;
		$$->next = $3;
	}
	| OP
	{
		$$ = calloc(1, sizeof(*$$));
		$$->t  = 1;
		$$->op = $1.v;

		free($1.s);
	}
	| I64
	{
		$$ = calloc(1, sizeof(*$$));
		$$->t = 2;

		$$->arg = calloc(1, sizeof(*$$->arg));		
		arg * A = $$->arg;

		A->l = $1.l;
		A->s = $1.s;

		A->itype = ITYPE_I64;
		A->i64 = $1.v;
	}
	| string
	;

string
	: string STR
	{
		$$ = $1;

		arg * A = $$->arg;
		char* o = A->s;

		// reallocate the string value of the argument
		A->s = realloc(A->s, A->l + $2.l + 1);
		memcpy(A->s + A->l, $2.s, $2.l);
		A->s[A->l + $2.l] = 0;

		// update string pointers on previous references
		int x;
		for(x = 0; x < A->refsl; x++)
		{
			A->refs[x].s = A->s + (A->refs[x].s - o);
			A->refs[x].e = A->refs[x].s + A->refs[x].l;
		}

		A->l += $2.l;
		free($2.s);
	}
	| string BREF
	{
		$$ = $1;

		arg * A = $$->arg;
		char* o = A->s;

		// reallocate the string value of the argument
		A->s = realloc(A->s, A->l + $2.l + 1);
		memcpy(A->s + A->l, $2.s, $2.l);
		A->s[A->l + $2.l] = 0;

		// update string pointers on previous references
		int x;
		for(x = 0; x < A->refsl; x++)
		{
			A->refs[x].s = A->s + (A->refs[x].s - o);
			A->refs[x].e = A->refs[x].s + A->refs[x].l;
		}

		// use new reference, if there is one
		A->refs = realloc(A->refs, (A->refsl + 1) * sizeof(*A->refs));

		A->refs[A->refsl].s = A->s + A->l;
		A->refs[A->refsl].l = $2.l;
		A->refs[A->refsl].e = A->s + A->l + $2.l;
		A->refs[A->refsl].k = REFTYPE_BREF;
		A->refs[A->refsl].bref = $2.v;

		A->ref_last = &A->refs[A->refsl];
		A->refsl++;

		A->l += $2.l;
		free($2.s);
	}
	| STR
	{
		$$ = calloc(1, sizeof(*$$));
		$$->t = 2;

		$$->arg = calloc(1, sizeof(*$$->arg));
		arg * A = $$->arg;

		A->s = $1.s;
		A->l = $1.l;
	}
	| BREF
	{
		$$ = calloc(1, sizeof(*$$));
		$$->t = 2;

		$$->arg = calloc(1, sizeof(*$$->arg));
		arg * A = $$->arg;

		A->s = $1.s;
		A->l = $1.l;

		A->refs = malloc(sizeof(*A->refs));

		A->refs[0].s = A->s;
		A->refs[0].l = A->l;
		A->refs[0].e = A->s + A->l;
		A->refs[0].k = REFTYPE_BREF;
		A->refs[0].bref = $1.v;

		A->ref_last = &A->refs[0];
		A->refsl = 1;
	}
	|
	{
		$$ = calloc(1, sizeof(*$$));
		$$->t = 2;

		$$->arg = calloc(1, sizeof(*$$->arg));
		arg * A = $$->arg;
		A->s = strdup("");
	}
	;
