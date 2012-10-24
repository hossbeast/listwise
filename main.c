#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <listwise.h>
#include <listwise/operator.h>
#include <listwise/ops.h>
#include <listwise/generator.h>
#include <listwise/lstack.h>

#define FAIL(x, ...) { printf(x, ##__VA_ARGS__); return 1; }

struct
{
	char		dump;				// -d
	char		number;			// -n
	char		in_null;		// -0
	char		out_null;		// -z
} args;

static void usage()
{
	printf(
		"listwise : list-centric computation utility\n"
		"\n"
		"Usage : lw [options]\n"
		"  --help|-h for this message\n"
		"\n"
		"------------------[options]----------------------------------------\n"
		"\n"
		" -d    dump list-stack at each step during execution\n"
		" -n    number output items\n"
		" -z    separate output items with null byte instead of newline\n"
		" -0    read input items separated by null byte instead of newline\n"
		"\n"
		"------------------[operators]--------------------------------------\n"
		"\n"
	);

	int x;
	for(x = 0; x < g_ops_l; x++)
	{
		printf("%s\n", g_ops[x]->desc);
	}

	printf(
		"\n"
	);
}

int main(int argc, char** argv)
{
	int x;

	// generator parser
	generator_parser* p = 0;

	// generator
	generator* g = 0;

	// list stack
	lstack* ls = 0;

	int* ovec = 0;
	int ovec_len = 0;

	if(generator_mkparser(&p) == 0)
		FAIL("mkparser failed\n");

	if(argc > 1)
	{
		if(generator_parse(p, argv[1], 0, &g) == 0)
			FAIL("parse failed\n");

		if(args.dump)
		{
			printf("GENERATOR: \n");
			generator_dump(g);
		}

		if((ls = calloc(1, sizeof(*ls))) == 0)
			FAIL("calloc failed\n");

		// write init elements to top of list stack
		for(x = 0; x < g->argsl; x++)
		{
			if(lstack_write(ls, 0, x, g->args[x]->s, g->args[x]->l) == 0)
				FAIL("lstack_write failed\n");
		}

		// execute operators
		for(x = 0; x < g->opsl; x++)
		{
			if(args.dump)
			{
				printf(" >> %s", g->ops[x]->op->s);

				int y;
				for(y = 0; y < g->ops[x]->argsl; y++)
					printf("/%s", g->ops[x]->args[y]->s);

				printf("\n");
			}

			if(g->ops[x]->op->op_exec(g->ops[x], ls, &ovec, &ovec_len) == 0)
				FAIL("operator exec failed\n");

			if(args.dump)
			{
				lstack_dump(ls);
			}
		}

		// OUTPUT

		if(ls->sel.l)
		{
			for(x = 0; x < ls->sel.l; x++)
			{
				if(args.number)
					printf("%3d %.*s", x, ls->s[0].s[ls->sel.s[x]].l, ls->s[0].s[ls->sel.s[x]].s);
				else
					printf("%.*s", ls->s[0].s[ls->sel.s[x]].l, ls->s[0].s[ls->sel.s[x]].s);

				if(args.out_null)
					printf("%hhu", 0);
				else
					printf("\n");
			}
		}
		else if(ls->l)
		{
			for(x = 0; x < ls->s[0].l; x++)
			{
				if(args.number)
					printf("%3d %.*s", x, ls->s[0].s[x].l, ls->s[0].s[x].s);
				else
					printf("%.*s", ls->s[0].s[x].l, ls->s[0].s[x].s);

				if(args.out_null)
					printf("%hhu", 0);
				else
					printf("\n");
			}
		}
	}

	free(ovec);
	lstack_xfree(&ls);
	generator_xfree(&g);
	generator_parser_xfree(&p);

	return 0;
}
