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

#ifndef YY_YY_REGEX_H_INCLUDED
#define YY_YY_REGEX_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 10 "regex.y" /* yacc.c:1909  */

#include "regex_tree.h"

#line 48 "regex.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype {
    CHAR = 258,
    CNUM = 259,
    SUBMATCH = 260,
    ALPH = 261,
    CAPT = 262,
    DIGIT = 263,
    INT = 264,
    DOUBLE = 265,
    TEXT = 266
};
#endif
/* Tokens.  */
#define CHAR 258
#define CNUM 259
#define SUBMATCH 260
#define ALPH 261
#define CAPT 262
#define DIGIT 263
#define INT 264
#define DOUBLE 265
#define TEXT 266

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED

union YYSTYPE {
#line 17 "regex.y" /* yacc.c:1909  */

    NFA *nfa;
    int ivalue;
    uint8_t ucvalue;  // todo: UTF-8
    char cvalue;
    char *str;
    uint8_t *charsets;  // size: 256

#line 91 "regex.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

int yyparse(void);

#endif /* !YY_YY_REGEX_H_INCLUDED  */
