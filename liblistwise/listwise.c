#include "listwise/internal.h"
#include "control.h"

int listwise_exec(char* s, int l, char** init, int* initls, int initl, lstack** ls)
{
	// generator parser
	generator_parser* p = 0;

	// generator
	generator* g = 0;

	if(generator_mkparser(&p) == 0)
		FAIL("mkparser failed\n");

	if(generator_parse(p, s, l, &g) == 0)
		FAIL("parse failed\n");

	if(lstack_exec(g, init, initls, initl, ls) == 0)
		FAIL("lstack_exec failed");

	generator_free(g);
	generator_parser_free(p);

	return 1;
}
