CC= gcc
CFLAGS= -c -O3

lex.regex.o:
	lex -o lex.regex.c regex.lex
	$(CC) $(CFLAGS) lex.regex.c -o $@

regex.o:
	yacc -d regex.y -o regex.c
	$(CC) $(CFLAGS) regex.c -o $@

regex_tree.o:
	$(CC) $(CFLAGS) regex_tree.c -o $@
