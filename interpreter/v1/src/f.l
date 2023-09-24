%{
#include <stdio.h>
#include "f.tab.h"
#define LEXER_LOG_PREFIX [Lexer]
int yyparse();
%}

integer [0-9][0-9]+
real [0-9][0-9]+\.[0-9][0-9]+


letter ([a-zA-Z])
identifier (letter)(letter | digit)+

%%

break {printf("%s sf\n", yytext); return SF_BREAK;}
cond {printf("%s sf\n", yytext); return SF_COND;}
func {printf("%s sf\n", yytext); return SF_FUNC;}
lambda {printf("%s sf\n", yytext); return SF_LAMBDA;}
prog {printf("%s sf\n", yytext); return SF_PROG;}
quote {printf("%s sf\n", yytext); return SF_QOUTE;}
return {printf("%s sf\n", yytext); return SF_RETURN;}
setq {printf("%s sf\n", yytext); return SF_SETQ;}
while {printf("%s sf\n", yytext); return SF_WHILE;}

plus {printf("%s sf\n", yytext); return PF_PLUS;}
times {printf("%s sf\n", yytext); return PF_TIMES;}
divide {printf("%s sf\n", yytext); return PF_DIVIDE;}
minus {printf("%s sf\n", yytext); return PF_MINUS;}

head {printf("%s sf\n", yytext); return PF_HEAD;}
tail {printf("%s sf\n", yytext); return PF_TAIL;}
cons {printf("%s sf\n", yytext); return PF_CONS;}

equal {printf("%s sf\n", yytext); return PF_EQUAL;}
nonequal {printf("%s sf\n", yytext); return PF_NONEQUAL;}
less {printf("%s sf\n", yytext); return PF_LESS;}
lesseq {printf("%s sf\n", yytext); return PF_LESSEQ;}
greater {printf("%s sf\n", yytext); return PF_GREATER;}
greatereq {printf("%s sf\n", yytext); return PF_GREATEREQ;}

isint {printf("%s sf\n", yytext); return PF_ISINT;}
isreal {printf("%s sf\n", yytext); return PF_ISREAL;}
isbool {printf("%s sf\n", yytext); return PF_ISBOOL;}
isnull {printf("%s sf\n", yytext); return PF_ISNULL;}
isatom {printf("%s sf\n", yytext); return PF_ISATOM;}
islist {printf("%s sf\n", yytext); return PF_ISLIST;}

and {printf("%s sf\n", yytext); return PF_AND;}
or {printf("%s sf\n", yytext); return PF_OR;}
xor {printf("%s sf\n", yytext); return PF_XOR;}
not {printf("%s sf\n", yytext); return PF_NOT;}

eval {printf("%s sf\n", yytext); return PF_EVAL;}

true |
false |
null |
[+|-]?[0-9]+[\.][0-9]+ |
[+|-]?[0-9]+ {printf("%s literal\n", yytext); return LITERAL;}

[a-zA-Z][a-zA-Z0-9]* {printf("%s identifier\n", yytext); return IDENTIFIER;}

[ ] {return SYM_SPACE;}
[\t] {return SYM_TAB;}
[\n] {return EOL;}
. {printf("%s got\n", yytext); return yytext[0];}
%%

int yywrap() {
	return 1;
}
