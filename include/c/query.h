/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_QUERY_H_INCLUDED
# define YY_YY_QUERY_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 9 "query.y" /* yacc.c:1909  */

#include "query_tree.h"

#line 48 "query.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    DOUBLE_TK = 258,
    INT_TK = 259,
    IDENTIFIER_TK = 260,
    STRING_TK = 261,
    SELECT_TK = 262,
    SUM_TK = 263,
    COUNT_TK = 264,
    AVG_TK = 265,
    MIN_TK = 266,
    MAX_TK = 267,
    DISTINCT_TK = 268,
    AS_TK = 269,
    FROM_TK = 270,
    WHERE_TK = 271,
    AND_TK = 272,
    OR_TK = 273,
    IS_TK = 274,
    NOT_TK = 275,
    NIL_TK = 276,
    BETWEEN_TK = 277,
    REGEXP_TK = 278,
    LIKE_TK = 279,
    EQ_TK = 280,
    NOTEQ_TK = 281,
    LESS_TK = 282,
    GREAT_TK = 283,
    LESSEQ_TK = 284,
    GREATEQ_TK = 285,
    GROUP_TK = 286,
    BY_TK = 287,
    ORDER_TK = 288,
    LIMIT_TK = 289
  };
#endif
/* Tokens.  */
#define DOUBLE_TK 258
#define INT_TK 259
#define IDENTIFIER_TK 260
#define STRING_TK 261
#define SELECT_TK 262
#define SUM_TK 263
#define COUNT_TK 264
#define AVG_TK 265
#define MIN_TK 266
#define MAX_TK 267
#define DISTINCT_TK 268
#define AS_TK 269
#define FROM_TK 270
#define WHERE_TK 271
#define AND_TK 272
#define OR_TK 273
#define IS_TK 274
#define NOT_TK 275
#define NIL_TK 276
#define BETWEEN_TK 277
#define REGEXP_TK 278
#define LIKE_TK 279
#define EQ_TK 280
#define NOTEQ_TK 281
#define LESS_TK 282
#define GREAT_TK 283
#define LESSEQ_TK 284
#define GREATEQ_TK 285
#define GROUP_TK 286
#define BY_TK 287
#define ORDER_TK 288
#define LIMIT_TK 289

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 16 "query.y" /* yacc.c:1909  */

    Node *node;
	double dvalue;
    int ivalue;
	char *str;

#line 135 "query.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_QUERY_H_INCLUDED  */
