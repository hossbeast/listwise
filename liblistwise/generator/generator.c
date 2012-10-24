#include "listwise/internal.h"

#include "generator.h"
#include "generator.def.h"
#include "generator.tab.h"
#include "generator.lex.h"

#include "re.h"

// defined in the bison parser
int generator_yyparse(yyscan_t, parse_param*);

int API generator_mkparser(generator_parser** p)
{
	yyscan_t* scanner = p;
	return generator_yylex_init(scanner) ? 0 : 1;
}

int API generator_parse(generator_parser* p, char* s, int l, generator** g)
{
	// create state specific to this parse
	void* state = 0;
	if((state = generator_yy_scan_string(s, p)) == 0)
	{
		return 0;
	}

	// allocate generator
	(*g) = calloc(1, sizeof(*g[0]));

	// results struct for this parse
	parse_param pp = {
		  .r = 1
		, .g = *g
	};

	// make it available to the lexer
	generator_yyset_extra(&pp, p);

	// parse
	generator_yyparse(p, &pp);

	// cleanup state for this parse
	generator_yy_delete_buffer(state, p);

	// postprocessing
	int x;
	for(x = 0; x < (*g)->opsl; x++)
	{
		if((*g)->ops[x]->op->op_validate((*g)->ops[x]) == 0)
			return 0;
	}

	return pp.r;
}

void API generator_parser_free(generator_parser* p)
{
	generator_yylex_destroy(p);
}

void API generator_parser_xfree(generator_parser** p)
{
	if(p)
		generator_parser_free(*p);

	*p = 0;
}

void generator_yyerror(yyscan_t scanner, parse_param* pp, char const *err)
{
	printf("ERROR - %s\n", err);
	pp->r = 0;
}

void API generator_dump(generator* g)
{
	int x, y;

	char s[256] = { [0] = 0 };

	printf("generator @ %p {\n", g);
	printf("  initial list\n");
	for(x = 0; x < g->argsl; x++)
	{
		sprintf(s + strlen(s), "%.*s/", g->args[x]->l, g->args[x]->s);

		printf("    '%.*s'\n", g->args[x]->l, g->args[x]->s);
	}

	printf("  operations\n");
	for(x = 0; x < g->opsl; x++)
	{
		if(x)
			sprintf(s + strlen(s), "/");
		sprintf(s + strlen(s), "%s", g->ops[x]->op->s);

		printf("    OP - %s\n", g->ops[x]->op->s);
		printf("      args\n");
		for(y = 0; y < g->ops[x]->argsl; y++)
		{
			sprintf(s + strlen(s), "/");
			sprintf(s + strlen(s), "%s", g->ops[x]->args[y]->s);

			printf("        '%s'\n", g->ops[x]->args[y]->s);
		}

		if(y == 0)
			printf("        none\n");
	}

	if(x == 0)
		printf("    none\n");

	printf("\n");
	printf(" --> %s\n", s);
	printf("}\n");
}

void API generator_free(generator* g)
{
	if(g)
	{
		int x;
		for(x = 0; x < g->argsl; x++)
		{
			free(g->args[x]->s);
			free(g->args[x]->refs);

			if(g->args[x]->itype)
			{
				free(g->args[x]->re.c_pcre);
				free(g->args[x]->re.c_pcre_extra);
			}

			free(g->args[x]);
		}

		free(g->args);

		for(x = 0; x < g->opsl; x++)
		{
			int y;
			for(y = 0; y < g->ops[x]->argsl; y++)
			{
				free(g->ops[x]->args[y]->s);
				free(g->ops[x]->args[y]->refs);

				if(g->ops[x]->args[y]->itype)
				{
					free(g->ops[x]->args[y]->re.c_pcre);
					free(g->ops[x]->args[y]->re.c_pcre_extra);
				}

				free(g->ops[x]->args[y]);
			}

			free(g->ops[x]->args);
			free(g->ops[x]);
		}

		free(g->ops);
	}

	free(g);
}

void API generator_xfree(generator** g)
{
	generator_free(*g);
	*g = 0;
}
