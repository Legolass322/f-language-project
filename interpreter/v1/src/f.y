%{
#include <stdio.h>
#define LOG_PREFIX [Parser]
int yylex(void);
int yyerror(const char * s) {
  printf("LOG_PREFIX Error: %s\n", s);
  return 0;
}
%}

%locations

%token SF_BREAK SF_COND SF_FUNC SF_LAMBDA SF_PROG SF_QOUTE SF_RETURN SF_SETQ SF_WHILE
%token PF_PLUS PF_TIMES PF_DIVIDE PF_MINUS
%token PF_HEAD PF_TAIL PF_CONS
%token PF_EQUAL PF_NONEQUAL PF_LESS PF_LESSEQ PF_GREATER PF_GREATEREQ
%token PF_ISINT PF_ISREAL PF_ISBOOL PF_ISNULL PF_ISATOM PF_ISLIST
%token PF_AND PF_OR PF_XOR PF_NOT
%token PF_EVAL
%token IDENTIFIER
%token LITERAL
%token SYM_SPACE SYM_TAB EOL 
%left '(' ')'

%%

program: 
  elements {printf("LOG_PREFIX program\n");}

elements:
  | element del elements {printf("LOG_PREFIX elements\n");}

element:
  atom | list | literal 
  {printf("LOG_PREFIX element\n"); }

list: 
  '(' del SF_QOUTE del element del ')' |
  '(' del SF_SETQ del atom del element del ')' |
  '(' del SF_FUNC del atom del list del element del ')' |
  '(' del SF_LAMBDA del list del element del ')' |
  '(' del SF_PROG del list del element del ')' |
  '(' del SF_COND del element del element del element del ')' |
  '(' del SF_COND del element del element del ')' |
  '(' del SF_WHILE del element del element del ')' |
  '(' del SF_RETURN del element del ')' |
  '(' del SF_BREAK del ')' |

  '(' del PF_PLUS del element del element del ')' |
  '(' del PF_MINUS del element del element del ')' |
  '(' del PF_DIVIDE del element del element del ')' |
  '(' del PF_TIMES del element del element del ')' |

  '(' del PF_HEAD del element del ')' |
  '(' del PF_TAIL del element del ')' |
  '(' del PF_CONS del element del element del ')' |

  '(' del PF_EQUAL del element del element del ')' |
  '(' del PF_NONEQUAL del element del element del ')' |
  '(' del PF_LESS del element del element del ')' |
  '(' del PF_LESSEQ del element del element del ')' |
  '(' del PF_GREATEREQ del element del element del ')' |
  '(' del PF_GREATER del element del element del ')' |

  '(' del PF_ISINT del element del ')' |
  '(' del PF_ISREAL del element del ')' |
  '(' del PF_ISBOOL del element del ')' |
  '(' del PF_ISNULL del element del ')' |
  '(' del PF_ISATOM del element del ')' |
  '(' del PF_ISLIST del element del ')' |

  '(' del PF_AND del element del element del ')' |
  '(' del PF_OR del element del element del ')' |
  '(' del PF_XOR del element del element del ')' |
  '(' del PF_NOT del element del ')' |

  '(' del IDENTIFIER del elements del ')' |

  '(' del PF_EVAL del element del ')' |

  '(' del elements del ')'


atom:
  IDENTIFIER {printf("LOG_PREFIX atom\n");}

literal:
  LITERAL {printf("LOG_PREFIX literal\n");}

del:
  | delimiter_spaces_all del
delimiter_spaces_all:
  | spaces | tabs | eols
spaces:
  | SYM_SPACE spaces
tabs:
  | SYM_TAB tabs
eols:
  | EOL eols

%%

int main() {
  if (yyparse()) {
    printf("LOG_PREFIX ERROR\n");
    return 1;
  }
  return 0;
}
