%{
	#include <stdio.h>

	#include "generator.def.h"
	#include "generator.tab.h"

	void generator_yyerror(void* scanner, parse_param* pp, char const* err);

#define PTOKEN(token)																																								\
		token == OP ?   printf("%3d: OP   - %s (%p)\n", __LINE__, yylval->op->s, yylval->op)						\
	: token == STR ?  printf("%3d: STR  - '%.*s'\n", __LINE__, yylval->strseg.l, yylval->strseg.s)		\
	: token == BREF ? printf("%3d: BREF - '%.*s'\n", __LINE__, yylval->strseg.l, yylval->strseg.s)		\
	: token == VREF ? printf("%3d: VREF - '%.*s'\n", __LINE__, yylval->strseg.l, yylval->strseg.s)		\
	:                 printf("%3d: TOK  - '%c' (%d)\n", __LINE__, token, token)

#if 0
#	define LEX(token) PTOKEN(token), token
#else
#	define LEX(token) token
#endif
%}

%option nounput noyywrap
%option reentrant
%option bison-bridge 
%option prefix="generator_yy"
%option extra-type="parse_param*"

%x arg1 arg2 arg3

%%

[/]							{ return LEX(yytext[0]); }
\\[0-9]+				{
									yylval->strseg.s = strdup(yytext);
									yylval->strseg.l = yyleng;
									return LEX(BREF);
								}
\$\{[a-zA-Z_][a-zA-Z0-9_]*\} {
									yylval->strseg.s = strdup(yytext);
									yylval->strseg.l = yyleng;
									return LEX(VREF);
								}
[^\\/<]+				{
									// determine whether this string happens to be an operator
									if((yylval->op = op_lookup(yytext, yyleng)))
									{
										return LEX(OP);
									}
									else
									{
										yylval->strseg.s = strdup(yytext);
										yylval->strseg.l = yyleng;
										return LEX(STR);
									}
								}
"<"							{
									/* literal string */
									yylval->strseg.s = 0;
									yylval->strseg.l = 0;
									int a = 0;
									int n = 0;

									while(1)
									{
										int c = input(yyscanner);
										if(!n && c == '\\')
										{
											n++;
										}
										else
										{
											if(n && (c != '\\' && c != '>'))
											{
												// backslash followed by illegal character
											}
											else
											{
												if(c == EOF || (c == '>' && !n))
												{
													break;
												}
												else if(yylval->strseg.l == a)
												{
													yylval->strseg.s = realloc(yylval->strseg.s, yylval->strseg.l + 11);
													a += 10;
												}
												yylval->strseg.s[yylval->strseg.l++] = c;
											}

											n = 0;
										}
									}
				
									return LEX(STR);
								}
