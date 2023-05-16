# Compiler
CC = g++
CFLAGS = -c -std=c++17 -fPIC -mavx512f -mavx512bw -mavx512vl -mavx512dq

EXECFLAGS = -Wall
EXECFLAGS += -O3 -DBENCH 
# EXECFLAGS += -g
# EXECFLAGS += -O0 -g -Wextra -DDEBUG -DBENCH

LDFLAGS = -shared
# BENCHFLAGS = -DBENCH
INCLUDES = -I../include -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
TARGET = libvlex.so
ODIR = obj
SDIR = ../src

_OBJS = edu_utokyo_vlex_VlexNative.o charcode.o command.o command-node.o command-parser.o command-scanner.o dfa.o executor.o ios.o optimizer.o pfa.o query-node.o query-parser.o query-scanner.o regex-node.o regex-parser.o regex-scanner.o runtime.o vfa.o
OBJS = $(patsubst %, $(ODIR)/%, $(_OBJS))

GEN_CODES = $(SDIR)/command-parser.cpp $(SDIR)/command-scanner.cpp $(SDIR)/regex-parser.cpp $(SDIR)/regex-scanner.cpp $(SDIR)/query-parser.cpp $(SDIR)/query-scanner.cpp

all: $(TARGET)

$(SDIR)/command-parser.cpp: ../generator/command.ypp
	bison -d -o command-parser.cpp $<
	(mv -f command-parser.cpp $(SDIR)/; mv -f command-parser.hpp ../include/parser/cmd/;)

$(SDIR)/command-scanner.cpp: ../generator/command.lex
	flex -8 -o command-scanner.cpp $<
	(mv -f command-scanner.cpp $(SDIR)/)

$(SDIR)/regex-parser.cpp: ../generator/regex.ypp
	bison -d -o regex-parser.cpp $<
	(mv -f regex-parser.cpp $(SDIR)/; mv -f regex-parser.hpp ../include/parser/regex/;)

$(SDIR)/regex-scanner.cpp: ../generator/regex.lex
	flex -8 -o regex-scanner.cpp $<
	(mv -f regex-scanner.cpp $(SDIR)/)

$(SDIR)/query-parser.cpp: ../generator/query.ypp
	bison -d -o query-parser.cpp $<
	(mv -f query-parser.cpp $(SDIR)/; mv -f query-parser.hpp ../include/parser/query/;)

$(SDIR)/query-scanner.cpp: ../generator/query.lex
	flex -8 -o query-scanner.cpp $<
	(mv -f query-scanner.cpp $(SDIR)/)

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) $(EXECFLAGS) $(INCLUDES) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(INCLUDES) -o $(TARGET) $(LIBRARIES) $(LDFLAGS) $^

$(ODIR)/edu_utokyo_vlex_VlexNative.o: edu_utokyo_vlex_VlexNative.cpp $(GEN_CODES)
	mkdir -p $(ODIR) && $(CC) $(CFLAGS) $(EXECFLAGS) $(INCLUDES) -o $@ $<

edu_utokyo_vlex_VlexNative.cpp: target/classes/edu/utokyo/vlex/VlexNative.class
	javah -cp target/classes edu.utokyo.vlex.VlexNative

target/classes/edu/utokyo/vlex/VlexNative.class:
	mvn package

run:
	bash ./run.sh --local

test:
	make && make run

query:
	mvn clean && make test

clean:
	mvn clean && rm -rf obj/*.o *.class $(TARGET)