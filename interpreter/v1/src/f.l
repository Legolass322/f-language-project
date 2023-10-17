%{
#include <stdio.h>
#include "f.tab.h"
#define LEXER_LOG_PREFIX [Lexer]
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc.last_line++; \
            yylloc.last_column = 0; \
        } \
        else { \
            yylloc.last_column++; \
        } \
    }
int yyparse();
%}

integer [0-9][0-9]+
real [0-9][0-9]+\.[0-9][0-9]+


letter ([a-zA-Z])
identifier (letter)(letter | digit)+

%option yylineno

%%

break {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_BREAK;}
cond {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_COND;}
func {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_FUNC;}
lambda {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_LAMBDA;}
prog {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_PROG;}
quote {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_QOUTE;}
return {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_RETURN;}
setq {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_SETQ;}
while {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return SF_WHILE;}

plus {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_PLUS;}
times {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_TIMES;}
divide {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_DIVIDE;}
minus {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_MINUS;}

head {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_HEAD;}
tail {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_TAIL;}
cons {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_CONS;}

equal {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_EQUAL;}
nonequal {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_NONEQUAL;}
less {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_LESS;}
lesseq {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_LESSEQ;}
greater {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_GREATER;}
greatereq {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_GREATEREQ;}

isint {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_ISINT;}
isreal {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_ISREAL;}
isbool {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_ISBOOL;}
isnull {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_ISNULL;}
isatom {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_ISATOM;}
islist {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_ISLIST;}

and {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_AND;}
or {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_OR;}
xor {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_XOR;}
not {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_NOT;}

eval {printf("%s %d:%d sf\n", yytext, yylloc.first_line, yylloc.first_column); return PF_EVAL;}

true |
false |
null |
[+|-]?[0-9]+[\.][0-9]+ |
[+|-]?[0-9]+ {printf("%s %d:%d literal\n", yytext, yylloc.first_line, yylloc.first_column); return LITERAL;}

[a-zA-Z][a-zA-Z0-9]* {printf("%s %d:%d identifier\n", yytext, yylloc.first_line, yylloc.first_column); return IDENTIFIER;}

[ ] {return SYM_SPACE;}
[\t] {return SYM_TAB;}
[\n] {return EOL;}
. {printf("%s %d:%d del\n", yytext, yylloc.first_line, yylloc.first_column); return yytext[0];}
%%

int yywrap() {
	return 1;
}
