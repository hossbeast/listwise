%{
	#include <stdio.h>

	#include "generator.def.h"
	#include "generator.tab.h"

	void generator_yyerror(void* scanner, parse_param* pp, char const* err);

#define PTOKEN(token)																																													\
		token == OP ?   printf("%3d: OP   - '%.*s' (%p)\n", __LINE__, yylval->str.l, yylval->str.s, yylval->op.v)	\
	: token == STR ?  printf("%3d: STR  - '%.*s'\n"			, __LINE__, yylval->str.l, yylval->str.s)								\
	: token == I64 ?  printf("%3d: I64  - '%.*s'\n"			, __LINE__, yylval->str.l, yylval->str.s)								\
	: token == BREF ? printf("%3d: BREF - '%.*s'\n"			, __LINE__, yylval->str.l, yylval->str.s)								\
	:                 printf("%3d: TOK  - (%c) (%d)\n"	, __LINE__, token, token)

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

\\[/]						{
									yylval->str.s = strdup("/");
									yylval->str.l = 1;
									return LEX(STR);
								}
[/]							{ return LEX(yytext[0]); }
\\[0-9]+				{
									yylval->bref.s = strdup(yytext);
									yylval->bref.l = yyleng;
									sscanf(yytext + 1, "%d", &yylval->bref.v);
									return LEX(BREF);
								}
-?[0-9]+				{
									yylval->i64.s = strdup(yytext);
									yylval->i64.l = yyleng;
									sscanf(yytext, "%ld", &yylval->i64.v);
									return LEX(I64);
								}
[^\\/<]+				{
									yylval->str.s = strdup(yytext);
									yylval->str.l = yyleng;

									// determine whether this string happens to be an operator
									if((yylval->op.v = op_lookup(yytext, yyleng)))
										return LEX(OP);
									else
										return LEX(STR);
								}
"<"							{
									/* literal string */
									yylval->str.s = 0;
									yylval->str.l = 0;
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
												else if(yylval->str.l == a)
												{
													yylval->str.s = realloc(yylval->str.s, yylval->str.l + 11);
													a += 10;
												}
												yylval->str.s[yylval->str.l++] = c;
											}

											n = 0;
										}
									}
				
									return LEX(STR);
								}
