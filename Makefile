# Compiler
CC = g++
CFLAGS = -std=c++17 -flto -mavx512f -mavx512bw -mavx512vl -mavx512dq
# CFLAGS = -std=c++17 -O3 -flto -march=native

# QEXECFLAGS += -DVECEXEC
# QEXECFLAGS += -DVECEXEC -DNOEXEC
# QEXECFLAGS += -DCODEGEN
EXECFLAGS = -O3
DEBUGFLAGS = -O0 -g
DEBUGFLAGS += -Wall -Wextra
BENCHFLAGS = -DBENCH
INCLUDES = -I./include -I./include/parser/query -I./include/parser/regex
FLAGS = $(CFLAGS) $(INCLUDES) $(QEXECFLAGS)

CPP_CODES = $(filter-out src/codegen.cpp, $(filter-out src/queryVExecutor.cpp, $(wildcard src/*.cpp))) main.cpp
CPP_PROGRAM = $(basename $(CPP_CODES))

ifeq ($(QEXECFLAGS), -DVECEXEC)
	CPP_CODES += src/queryVExecutor.cpp
endif

PROGRAM = vlex.exe

GEN = rparser rlexer qparser qlexer

all: $(PROGRAM)

$(PROGRAM): $(CPP_CODES)
	$(MAKE) $(GEN)
	$(CC) $(FLAGS) $(EXECFLAGS) $(BENCHFLAGS)  $^ -o $@

debug: $(CPP_CODES)
	$(MAKE) $(GEN)
	$(CC) $(FLAGS) $(DEBUGFLAGS) $^ -o vlex_debug.exe

rparser: generator/regex.ypp
	bison -d -oregexParser.cpp generator/regex.ypp
	(mv regexParser.cpp src/; mv regexParser.hpp include/parser/regex/;)

rlexer: generator/regex.lex
	flex -8 -oregexScanner.cpp generator/regex.lex
	(mv regexScanner.cpp src/)

qparser: generator/query.ypp
	bison -d -oqueryParser.cpp generator/query.ypp
	(mv queryParser.cpp src/; mv queryParser.hpp include/parser/query/;)

qlexer: generator/query.lex
	flex -8 -oqueryScanner.cpp generator/query.lex
	(mv queryScanner.cpp src/)

run:
	./$(PROGRAM)

clean:
	rm -f $(PROGRAM)
