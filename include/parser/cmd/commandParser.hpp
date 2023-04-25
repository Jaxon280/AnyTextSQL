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

#ifndef YY_COMMAND_YY_COMMANDPARSER_HPP_INCLUDED
# define YY_COMMAND_YY_COMMANDPARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef COMMAND_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define COMMAND_YYDEBUG 1
#  else
#   define COMMAND_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define COMMAND_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined COMMAND_YYDEBUG */
#if COMMAND_YYDEBUG
extern int command_yydebug;
#endif
/* "%code requires" blocks.  */
#line 24 "generator/command.ypp" /* yacc.c:1909  */

#include "parser/command.hpp"

#line 56 "commandParser.hpp" /* yacc.c:1909  */

/* Token type.  */
#ifndef COMMAND_YYTOKENTYPE
# define COMMAND_YYTOKENTYPE
  enum command_yytokentype
  {
    CEOF_TK = 0,
    CFILENAME_TK = 258,
    CIDENTIFIER_TK = 259,
    CPATTERN_TK = 260,
    CSCAN_TK = 261,
    CTABLE_TK = 262,
    CEXP_TK = 263,
    CKEYS_TK = 264
  };
#endif

/* Value type.  */
#if ! defined COMMAND_YYSTYPE && ! defined COMMAND_YYSTYPE_IS_DECLARED

union COMMAND_YYSTYPE
{
#line 29 "generator/command.ypp" /* yacc.c:1909  */

    char *svalue;
    vlex::StringList *slist;

#line 84 "commandParser.hpp" /* yacc.c:1909  */
};

typedef union COMMAND_YYSTYPE COMMAND_YYSTYPE;
# define COMMAND_YYSTYPE_IS_TRIVIAL 1
# define COMMAND_YYSTYPE_IS_DECLARED 1
#endif


extern COMMAND_YYSTYPE command_yylval;

int command_yyparse (vlex::CommandContext& ctx);
/* "%code provides" blocks.  */
#line 12 "generator/command.ypp" /* yacc.c:1909  */

typedef union COMMAND_YYSTYPE COMMAND_YYSTYPE;

#ifndef YY_DECL
#define YY_DECL int command_yylex(COMMAND_YYSTYPE &yylval)
YY_DECL;
#endif

int command_yyerror(vlex::CommandContext &ctx, const char *s);

#line 108 "commandParser.hpp" /* yacc.c:1909  */

#endif /* !YY_COMMAND_YY_COMMANDPARSER_HPP_INCLUDED  */
