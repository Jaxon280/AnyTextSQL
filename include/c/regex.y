%{
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "y.tab.h"
extern int yylex();
extern int yyerror();
%}
%code requires {
#include "regex_tree.h"
}
%code {
NFA *regex_root;
}
%union
{
    NFA *nfa;
    int ivalue;
    uint8_t ucvalue; // todo: UTF-8
    char cvalue;
	char *str;
    uint8_t *charsets; // size: 256
};
%token <cvalue> CHAR
%token <ivalue> CNUM
%token <str> '*' '(' ')' '.' '[' ']' '+' '-' '|' '%' '?' '^' SUBMATCH
%token <nfa> ALPH CAPT DIGIT INT DOUBLE TEXT
%type <nfa> regex union simple_re concat basic_re star plus select elementary_re int double text count
%type <charsets> charset setitems setitem range

%%

// https://www2.cs.sfu.ca/~cameron/Teaching/384/99-3/regexp-plg.html

line: regex { regex_root = $1; };

regex: union { $$ = $1; }
    | simple_re { $$ = $1; }
    ;

union: regex '|' simple_re { $$ = build_union_NFA($1, $3); };

simple_re: concat { $$ = $1; }
    | basic_re { $$ = $1; }
    ;

concat: simple_re basic_re { $$ = build_concat_NFA($1, $2); };

basic_re: star { $$ = $1; }
    | plus { $$ = $1; }
    | select { $$ = $1; }
    | elementary_re { $$ = $1; }
    | count { $$ = $1; }
    ;

star: elementary_re '*' { $$ = build_star_NFA($1); };

plus: elementary_re '+' { $$ = build_plus_NFA($1);};

select: elementary_re '?' { $$ = build_select_NFA($1);};

elementary_re: '(' regex ')' { $$ = $2; }
    | '(' SUBMATCH regex ')' { $$ = build_submatch_NFA($3, $2); }
    | '.' { $$ = build_wildcard_NFA(); }
    | DIGIT { $$ = build_digit_NFA(); }
    | ALPH { $$ = build_alph_NFA(); }
    | CAPT { $$ = build_capt_NFA(); }
    | CHAR { $$ = build_NFA($1); }
    | int { $$ = $1; }
    | double { $$ = $1; }
    | text { $$ = $1; }
    | charset { $$ = build_charsets_NFA($1); }
    ;

int: INT { $$ = build_INT(); };

double: DOUBLE { $$ = build_DOUBLE(); };

text: TEXT { $$ = build_TEXT(); };

count: elementary_re CNUM { $$ = build_num_NFA($1, $2); };

charset: '[' setitems ']' { $$ = $2; }
    | '[' '^' setitems ']' { $$ = negate_charsets($3); }
    ;

setitems: setitem { $$ = $1; }
    | setitem setitems { $$ = add_charsets($1, $2); }
    ;

setitem: range { $$ = $1; }
    | CHAR { $$ = build_c_charsets($1); }
    ;

range: CHAR '-' CHAR { $$ = build_range_charsets($1, $3); };

%%
int yyerror(char *s){
    printf("ERROR: %s\n\n",s);
  /*printf("\tERROR IN LINE %4d\n", yylval+1);
  */
	return *s;
}

void debug(NFA *nfa) {
    printf("Transition Size: %d\n", nfa->transSize);
    printf("State Size: %d\n", nfa->stateSize);
    printf("Init State: %d\n", nfa->initState);
    printf("Accept State: %d\n", nfa->acceptState);
    int sc;
    for (SubMatch *sm = nfa->subms; sm != NULL; sm = sm->next) {
        printf("Submatches[%d] name: %s, start: %d, end: %d\n", sc, sm->name, sm->start, sm->end);
        sc++;
    }
    for (int ti = 0; ti < nfa->transSize; ti++) {
        if (nfa->transVec[ti].c == -1) {
            printf("Transition[%d] start: %d, end: %d, char: EPSILON\n", ti, nfa->transVec[ti].start, nfa->transVec[ti].end);
        } else {
            printf("Transition[%d] start: %d, end: %d, char: %c\n", ti, nfa->transVec[ti].start, nfa->transVec[ti].end, (char)nfa->transVec[ti].c);
        }
    }
}

int main() {
    yyparse();
    debug(regex_root);
    return 0;
}
