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

#ifndef YY_REGEX_YY_REGEXPARSER_HPP_INCLUDED
# define YY_REGEX_YY_REGEXPARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef REGEX_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define REGEX_YYDEBUG 1
#  else
#   define REGEX_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define REGEX_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined REGEX_YYDEBUG */
#if REGEX_YYDEBUG
extern int regex_yydebug;
#endif
/* "%code requires" blocks.  */
#line 10 "generator/regex.ypp" /* yacc.c:1909  */

#include "parser/nfa.hpp"

#line 56 "regexParser.hpp" /* yacc.c:1909  */

/* Token type.  */
#ifndef REGEX_YYTOKENTYPE
# define REGEX_YYTOKENTYPE
  enum regex_yytokentype
  {
    REOF_TK = 0,
    RCHAR_TK = 258,
    RCNUM_TK = 259,
    RSUBMATCH_TK = 260,
    RALPH_TK = 261,
    RCAPT_TK = 262,
    RDIGIT_TK = 263,
    RINT_TK = 264,
    RDOUBLE_TK = 265
  };
#endif

/* Value type.  */
#if ! defined REGEX_YYSTYPE && ! defined REGEX_YYSTYPE_IS_DECLARED

union REGEX_YYSTYPE
{
#line 30 "generator/regex.ypp" /* yacc.c:1909  */

    vlex::NFA *nfa;
    int ivalue;
    uint8_t ucvalue; // todo: UTF-8
    char cvalue;
	char *svalue;
    uint8_t *charsets; // size: 256

#line 89 "regexParser.hpp" /* yacc.c:1909  */
};

typedef union REGEX_YYSTYPE REGEX_YYSTYPE;
# define REGEX_YYSTYPE_IS_TRIVIAL 1
# define REGEX_YYSTYPE_IS_DECLARED 1
#endif


extern REGEX_YYSTYPE regex_yylval;

int regex_yyparse (vlex::NFA** nfa);
/* "%code provides" blocks.  */
#line 18 "generator/regex.ypp" /* yacc.c:1909  */

typedef union REGEX_YYSTYPE REGEX_YYSTYPE;

#ifndef YY_DECL
#define YY_DECL int regex_yylex(REGEX_YYSTYPE &yylval)
YY_DECL;
#endif

int regex_yyerror(vlex::NFA **nfa, const char *s);

#line 113 "regexParser.hpp" /* yacc.c:1909  */

#endif /* !YY_REGEX_YY_REGEXPARSER_HPP_INCLUDED  */
