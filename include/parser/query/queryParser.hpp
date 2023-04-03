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

#ifndef YY_QUERY_YY_QUERYPARSER_HPP_INCLUDED
# define YY_QUERY_YY_QUERYPARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef QUERY_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define QUERY_YYDEBUG 1
#  else
#   define QUERY_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define QUERY_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined QUERY_YYDEBUG */
#if QUERY_YYDEBUG
extern int query_yydebug;
#endif
/* "%code requires" blocks.  */
#line 24 "../generator/query.ypp" /* yacc.c:1909  */

#include "parser/query.hpp"

#line 56 "queryParser.hpp" /* yacc.c:1909  */

/* Token type.  */
#ifndef QUERY_YYTOKENTYPE
# define QUERY_YYTOKENTYPE
  enum query_yytokentype
  {
    EOF_TK = 0,
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

/* Value type.  */
#if ! defined QUERY_YYSTYPE && ! defined QUERY_YYSTYPE_IS_DECLARED

union QUERY_YYSTYPE
{
#line 29 "../generator/query.ypp" /* yacc.c:1909  */

    vlex::OpTree *opt;
    vlex::PredTree *prt;
    vlex::Statement *stmt;
    vlex::StatementList *stmts;
    double dvalue;
    int ivalue;
    char *svalue;
    vlex::StringList *slist;

#line 115 "queryParser.hpp" /* yacc.c:1909  */
};

typedef union QUERY_YYSTYPE QUERY_YYSTYPE;
# define QUERY_YYSTYPE_IS_TRIVIAL 1
# define QUERY_YYSTYPE_IS_DECLARED 1
#endif


extern QUERY_YYSTYPE query_yylval;

int query_yyparse (vlex::QueryContext& ctx);
/* "%code provides" blocks.  */
#line 12 "../generator/query.ypp" /* yacc.c:1909  */

typedef union QUERY_YYSTYPE QUERY_YYSTYPE;

#ifndef YY_DECL
#define YY_DECL int query_yylex(QUERY_YYSTYPE &yylval)
YY_DECL;
#endif

int query_yyerror(vlex::QueryContext &ctx, const char *s);

#line 139 "queryParser.hpp" /* yacc.c:1909  */

#endif /* !YY_QUERY_YY_QUERYPARSER_HPP_INCLUDED  */
