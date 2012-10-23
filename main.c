#include <stdio.h>

#include <listwise.h>

#define FAIL(x, ...) { printf(x, ##__VA_ARGS__); return 1; }

int main(int argc, char** argv)
{
	// generator parser
	generator_parser* p = 0;

	// generator
	generator* g = 0;

	// list stack
	lstack* ls = 0;

	if(generator_mkparser(&p) == 0)
		FAIL("mkparser failed\n");

	if(argc > 1)
	{
		if(generator_parse(p, argv[1], 0, &g) == 0)
			FAIL("parse failed\n");

/*
		printf("GENERATOR: \n");
		generator_dump(g);
*/

		if(lstack_exec(g, 0, &ls) == 0)
			FAIL("list_exec failed");

/*
		printf("LIST-STACK\n");
		lstack_dump(ls);
*/

		if(ls->l)
		{
			int x;
			for(x = 0; x < ls->s[0].l; x++)
			{
				printf("%.*s\n", ls->s[0].s[x].l, ls->s[0].s[x].s);
			}
		}
	}

	lstack_xfree(&ls);
	generator_xfree(&g);
	generator_parser_xfree(&p);

	return 0;
}
