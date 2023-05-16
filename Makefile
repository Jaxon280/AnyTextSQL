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
INCLUDES = -I./include
FLAGS = $(CFLAGS) $(INCLUDES) $(QEXECFLAGS)
SDIR = src

CPP_CODES = $(filter-out src/codegen.cpp, $(filter-out src/query-vexecutor.cpp, $(wildcard src/*.cpp))) main.cpp

ifeq ($(QEXECFLAGS), -DVECEXEC)
	CPP_CODES += src/query-vexecutor.cpp
endif

PROGRAM = vlex.exe

GEN_CODES = $(SDIR)/command-parser.cpp $(SDIR)/command-scanner.cpp $(SDIR)/regex-parser.cpp $(SDIR)/regex-scanner.cpp $(SDIR)/query-parser.cpp $(SDIR)/query-scanner.cpp

all: $(PROGRAM)

$(PROGRAM): $(CPP_CODES) $(GEN_CODES)
	$(CC) $(FLAGS) $(EXECFLAGS) $(BENCHFLAGS)  $^ -o $@

debug: $(CPP_CODES) $(GEN_CODES)
	$(CC) $(FLAGS) $(DEBUGFLAGS) $^ -o vlex-debug.exe

$(SDIR)/command-parser.cpp: generator/command.ypp
	bison -d -o command-parser.cpp $<
	(mv -f command-parser.cpp $(SDIR)/; mv -f command-parser.hpp ../include/parser/cmd/;)

$(SDIR)/command-scanner.cpp: generator/command.lex
	flex -8 -o command-scanner.cpp $<
	(mv -f command-scanner.cpp $(SDIR)/)

$(SDIR)/regex-parser.cpp: generator/regex.ypp
	bison -d -o regex-parser.cpp $<
	(mv -f regex-parser.cpp $(SDIR)/; mv -f regex-parser.hpp ../include/parser/regex/;)

$(SDIR)/regex-scanner.cpp: generator/regex.lex
	flex -8 -o regex-scanner.cpp $<
	(mv -f regex-scanner.cpp $(SDIR)/)

$(SDIR)/query-parser.cpp: generator/query.ypp
	bison -d -o query-parser.cpp $<
	(mv -f query-parser.cpp $(SDIR)/; mv -f query-parser.hpp ../include/parser/query/;)

$(SDIR)/query-scanner.cpp: generator/query.lex
	flex -8 -o query-scanner.cpp $<
	(mv -f query-scanner.cpp $(SDIR)/)

run:
	./$(PROGRAM)

clean:
	rm -f $(PROGRAM)
