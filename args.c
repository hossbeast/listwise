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
		printf("%6s - %s\n", g_ops[x]->s, g_ops[x]->desc);
	}

	printf("\n");
	exit(!help);
}

int parse_args(int argc, char** argv)
{
	int c;
	while((c = getopt(argc, argv, "hdnz0")) != -1)
	{
		switch(c)
		{
			case 'd':
				g_args.dump = 1;
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
