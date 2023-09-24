/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SF_BREAK = 258,
     SF_COND = 259,
     SF_FUNC = 260,
     SF_LAMBDA = 261,
     SF_PROG = 262,
     SF_QOUTE = 263,
     SF_RETURN = 264,
     SF_SETQ = 265,
     SF_WHILE = 266,
     PF_PLUS = 267,
     PF_TIMES = 268,
     PF_DIVIDE = 269,
     PF_MINUS = 270,
     PF_HEAD = 271,
     PF_TAIL = 272,
     PF_CONS = 273,
     PF_EQUAL = 274,
     PF_NONEQUAL = 275,
     PF_LESS = 276,
     PF_LESSEQ = 277,
     PF_GREATER = 278,
     PF_GREATEREQ = 279,
     PF_ISINT = 280,
     PF_ISREAL = 281,
     PF_ISBOOL = 282,
     PF_ISNULL = 283,
     PF_ISATOM = 284,
     PF_ISLIST = 285,
     PF_AND = 286,
     PF_OR = 287,
     PF_XOR = 288,
     PF_NOT = 289,
     PF_EVAL = 290,
     IDENTIFIER = 291,
     LITERAL = 292,
     SYM_SPACE = 293,
     SYM_TAB = 294,
     EOL = 295
   };
#endif
/* Tokens.  */
#define SF_BREAK 258
#define SF_COND 259
#define SF_FUNC 260
#define SF_LAMBDA 261
#define SF_PROG 262
#define SF_QOUTE 263
#define SF_RETURN 264
#define SF_SETQ 265
#define SF_WHILE 266
#define PF_PLUS 267
#define PF_TIMES 268
#define PF_DIVIDE 269
#define PF_MINUS 270
#define PF_HEAD 271
#define PF_TAIL 272
#define PF_CONS 273
#define PF_EQUAL 274
#define PF_NONEQUAL 275
#define PF_LESS 276
#define PF_LESSEQ 277
#define PF_GREATER 278
#define PF_GREATEREQ 279
#define PF_ISINT 280
#define PF_ISREAL 281
#define PF_ISBOOL 282
#define PF_ISNULL 283
#define PF_ISATOM 284
#define PF_ISLIST 285
#define PF_AND 286
#define PF_OR 287
#define PF_XOR 288
#define PF_NOT 289
#define PF_EVAL 290
#define IDENTIFIER 291
#define LITERAL 292
#define SYM_SPACE 293
#define SYM_TAB 294
#define EOL 295




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

