/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_F_TAB_H_INCLUDED
# define YY_YY_F_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    SF_BREAK = 258,                /* SF_BREAK  */
    SF_COND = 259,                 /* SF_COND  */
    SF_FUNC = 260,                 /* SF_FUNC  */
    SF_LAMBDA = 261,               /* SF_LAMBDA  */
    SF_PROG = 262,                 /* SF_PROG  */
    SF_QOUTE = 263,                /* SF_QOUTE  */
    SF_RETURN = 264,               /* SF_RETURN  */
    SF_SETQ = 265,                 /* SF_SETQ  */
    SF_WHILE = 266,                /* SF_WHILE  */
    PF_PLUS = 267,                 /* PF_PLUS  */
    PF_TIMES = 268,                /* PF_TIMES  */
    PF_DIVIDE = 269,               /* PF_DIVIDE  */
    PF_MINUS = 270,                /* PF_MINUS  */
    PF_HEAD = 271,                 /* PF_HEAD  */
    PF_TAIL = 272,                 /* PF_TAIL  */
    PF_CONS = 273,                 /* PF_CONS  */
    PF_EQUAL = 274,                /* PF_EQUAL  */
    PF_NONEQUAL = 275,             /* PF_NONEQUAL  */
    PF_LESS = 276,                 /* PF_LESS  */
    PF_LESSEQ = 277,               /* PF_LESSEQ  */
    PF_GREATER = 278,              /* PF_GREATER  */
    PF_GREATEREQ = 279,            /* PF_GREATEREQ  */
    PF_ISINT = 280,                /* PF_ISINT  */
    PF_ISREAL = 281,               /* PF_ISREAL  */
    PF_ISBOOL = 282,               /* PF_ISBOOL  */
    PF_ISNULL = 283,               /* PF_ISNULL  */
    PF_ISATOM = 284,               /* PF_ISATOM  */
    PF_ISLIST = 285,               /* PF_ISLIST  */
    PF_AND = 286,                  /* PF_AND  */
    PF_OR = 287,                   /* PF_OR  */
    PF_XOR = 288,                  /* PF_XOR  */
    PF_NOT = 289,                  /* PF_NOT  */
    PF_EVAL = 290,                 /* PF_EVAL  */
    IDENTIFIER = 291,              /* IDENTIFIER  */
    LITERAL = 292,                 /* LITERAL  */
    SYM_SPACE = 293,               /* SYM_SPACE  */
    SYM_TAB = 294,                 /* SYM_TAB  */
    EOL = 295                      /* EOL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_F_TAB_H_INCLUDED  */
