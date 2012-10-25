#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <listwise.h>
#include <listwise/operator.h>
#include <listwise/ops.h>
#include <listwise/generator.h>
#include <listwise/lstack.h>

#include "args.h"

#define FAIL(x, ...) { printf(x "\n", ##__VA_ARGS__); return 1; }

int main(int argc, char** argv)
{
	int x = parse_args(argc, argv);

	// generator parser
	generator_parser* p = 0;

	// generator
	generator* g = 0;

	// list stack
	lstack* ls = 0;

	int* ovec = 0;
	int ovec_len = 0;

	if(generator_mkparser(&p) == 0)
		FAIL("mkparser failed");

	if(x < argc)
	{
		if(generator_parse(p, argv[x], 0, &g) == 0)
			FAIL("parse failed");

		if(g_args.dump)
		{
			printf("GENERATOR: \n");
			generator_dump(g);
		}

		if((ls = calloc(1, sizeof(*ls))) == 0)
			FAIL("calloc failed");

		// read from stdin if possible
		struct stat st;
		if(fstat(0, &st) == -1)
			FAIL("fstat(0) failed");

		if(S_ISREG(st.st_mode) || S_ISFIFO(st.st_mode))
		{
			void * mem = 0;
			size_t sz = 0;
			if(S_ISREG(st.st_mode))
			{
				sz = st.st_size;
				if((mem = mmap(0, sz, PROT_READ, MAP_PRIVATE, 0, 0)) == MAP_FAILED)
					FAIL("mmap(0) failed");
			}
			else if(S_ISFIFO(st.st_mode))
			{
				char blk[512];
				int r = 0;
				while((r = read(0, blk, sizeof(blk) / sizeof(blk[0]))) > 0)
				{
					mem = realloc(mem, sz + r);
					memcpy(((char*)mem) + sz, blk, r);
					sz += r;
				}
			}

			char * s[2] = { mem, 0 };
			while((s[1] = memchr(s[0], '\n', sz - (s[0] - ((char*)mem)))))
			{
				if(s[1] - s[0])
				{
					if(lstack_add(ls, s[0], s[1] - s[0]) == 0)
						FAIL("lstack_add failed");
				}
				s[0] = s[1] + 1;
			}

			if(S_ISREG(st.st_mode))
				munmap(mem, st.st_size);
		}

		// write init elements to top of list stack
		for(x = 0; x < g->argsl; x++)
		{
			if(lstack_add(ls, g->args[x]->s, g->args[x]->l) == 0)
				FAIL("lstack_add failed");
		}

		if(g_args.dump)
			lstack_dump(ls);

		// execute operators
		for(x = 0; x < g->opsl; x++)
		{
			if(g_args.dump)
			{
				printf(" >> %s", g->ops[x]->op->s);

				int y;
				for(y = 0; y < g->ops[x]->argsl; y++)
					printf("/%s", g->ops[x]->args[y]->s);

				printf("\n");
			}

			if(g->ops[x]->op->op_exec(g->ops[x], ls, &ovec, &ovec_len) == 0)
				FAIL("operator exec failed");

			if(g_args.dump)
				lstack_dump(ls);
		}

		// OUTPUT

		for(x = 0; x < ls->s[0].l; x++)
		{
			int go = 1;
			if(ls->sel.l)
			{
				if(ls->sel.sl <= (x/8))
					break;

				go = (ls->sel.s[x/8] & (0x01 << (x%8)));
			}

			if(go)
			{
				if(g_args.number)
					printf("%3d %.*s", x, ls->s[0].s[x].l, ls->s[0].s[x].s);
				else
					printf("%.*s", ls->s[0].s[x].l, ls->s[0].s[x].s);

				if(g_args.out_null)
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
