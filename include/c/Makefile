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

lex.query.o:
	lex -o lex.query.c query.lex
	$(CC) $(CFLAGS) lex.regex.c -o $@

query.o:
	yacc -d query.y -o query.c
	$(CC) $(CFLAGS) query.c -o $@

query_tree.o:
	$(CC) $(CFLAGS) query_tree.c -o $@

clean:
	rm $(wildcard *.o)
