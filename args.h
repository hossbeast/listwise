#ifndef _ARGS_H
#define _ARGS_H

struct g_args_t
{
	char		dump;				// -d
	char		number;			// -n
	char		in_null;		// -0
	char		out_null;		// -z
	char		out_stack;	// -k
	char		out_list;		// -a
} g_args;

int parse_args(int argc, char ** argv);

#endif
