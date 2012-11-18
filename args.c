#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "args.h"

#include <listwise.h>
#include <listwise/operator.h>
#include <listwise/ops.h>
#include <listwise/generator.h>
#include <listwise/lstack.h>

struct g_args_t g_args;

static void usage(int help)
{
	printf(
		"listwise : list-centric computation utility\n"
		"\n"
		"Usage : lw [options] generator-string\n"
		"  --help|-h for this message\n"
		"\n"
		"------------------[options]----------------------------------------\n"
		"\n"
		" -k    output entire stack, not just top list\n"
	 	"       implies -a\n"
		" -a    output entire list, not just selected entries\n"
		" -d    dump list-stack at each step during execution\n"
		" -n    number output items\n"
		" -z    separate output items with null byte instead of newline\n"
		" -0    read input items separated by null byte instead of newline\n"
		"\n"
		"------------------[operators]--------------------------------------\n"
		"\n"
		" 1  2  3  4  5  6  7  8  9    name  description\n"
	);

	int x;
	for(x = 0; x < g_ops_l; x++)
	{
		printf("[%c][%c][%c][%c][%c][%c][%c][%c][%c] %6s - %s\n"
			, g_ops[x]->optype & LWOP_SELECTION_READ				? 'x' : ' '
			, g_ops[x]->optype & LWOP_SELECTION_WRITE				? 'x' : ' '
			, g_ops[x]->optype & LWOP_SELECTION_RESET				? 'x' : ' '
			, g_ops[x]->optype & LWOP_MODIFIERS_CANHAVE			? 'x' : ' '
			, g_ops[x]->optype & LWOP_ARGS_CANHAVE					? 'x' : ' '
			, g_ops[x]->optype & LWOP_OPERATION_PUSHBEFORE	? 'x' : ' '
			, g_ops[x]->optype & LWOP_OPERATION_INPLACE			? 'x' : ' '
			, g_ops[x]->optype & LWOP_OPERATION_FILESYSTEM	? 'x' : ' '
			, g_ops[x]->optype & LWOP_OBJECT_NO							? 'x' : ' '
			, g_ops[x]->s
			, g_ops[x]->desc
		);
	}

	printf(
		" 1. SELECTION_READ\n"
		" 2. SELECTION_WRITE\n"
		" 3. SELECTION_RESET\n"
		" 4. MODIFIERS_CANHAVE\n"
		" 5. ARGS_CANHAVE\n"
		" 6. OPERATION_PUSHBEFORE\n"
		" 7. OPERATION_INPLACE\n"
		" 8. OPERATION_FILESYSTEM\n"
		" 9. OBJECT_NO\n"
	);

	exit(!help);
}

int parse_args(int argc, char** argv)
{
	int c;
	while((c = getopt(argc, argv, "adhknz0")) != -1)
	{
		switch(c)
		{
			case 'a':
				g_args.out_list = 1;
				break;
			case 'd':
				g_args.dump = 1;
				break;
			case 'k':
				g_args.out_stack = 1;
				g_args.out_list = 1;
				break;
			case 'n':
				g_args.number = 1;
				break;
			case 'z':
				g_args.out_null = 1;
				break;
			case '0':
				g_args.in_null = 1;
				break;
			default:
				usage(c == 'h');
		}
	}

	return optind;
}
