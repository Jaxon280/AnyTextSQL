# Compiler
CC = g++
CFLAGS = -c -std=c++17 -fPIC -mavx512f -mavx512bw -mavx512vl -mavx512dq

EXECFLAGS = -Wall
EXECFLAGS += -O3 -DBENCH 
# EXECFLAGS += -g
# EXECFLAGS += -O0 -g -Wextra -DDEBUG -DBENCH

LDFLAGS = -shared
# BENCHFLAGS = -DBENCH
INCLUDES = -I../include -I../include/parser/query -I../include/parser/regex -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
TARGET = libvlex.so
ODIR = obj
SDIR = ../src

_OBJS = edu_utokyo_vlex_VlexNative.o charCode.o command.o commandNode.o commandParser.o commandScanner.o dfa.o executor.o ios.o optimizer.o pfa.o queryNode.o queryParser.o queryScanner.o regexNode.o regexParser.o regexScanner.o runtime.o vfa.o
OBJS = $(patsubst %, $(ODIR)/%, $(_OBJS))

GEN = cparser clexer rparser rlexer qparser qlexer

all: $(TARGET)

gen:
	$(MAKE) $(GEN)

cparser: ../generator/command.ypp
	bison -d -ocommandParser.cpp ../generator/command.ypp
	(mv commandParser.cpp ../src/; mv commandParser.hpp ../include/parser/cmd/;)

clexer: ../generator/command.lex
	flex -8 -ocommandScanner.cpp ../generator/command.lex
	(mv commandScanner.cpp ../src/)

rparser: ../generator/regex.ypp
	bison -d -oregexParser.cpp ../generator/regex.ypp
	(mv regexParser.cpp ../src/; mv regexParser.hpp ../include/parser/regex/;)

rlexer: ../generator/regex.lex
	flex -8 -oregexScanner.cpp ../generator/regex.lex
	(mv regexScanner.cpp ../src/)

qparser: ../generator/query.ypp
	bison -d -oqueryParser.cpp ../generator/query.ypp
	(mv queryParser.cpp ../src/; mv queryParser.hpp ../include/parser/query/;)

qlexer: ../generator/query.lex
	flex -8 -oqueryScanner.cpp ../generator/query.lex
	(mv queryScanner.cpp ../src/)

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) $(EXECFLAGS) $(INCLUDES) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(INCLUDES) -o $(TARGET) $(LIBRARIES) $(LDFLAGS) $^

$(ODIR)/edu_utokyo_vlex_VlexNative.o: edu_utokyo_vlex_VlexNative.cpp
	mkdir -p $(ODIR) && $(CC) $(CFLAGS) $(EXECFLAGS) $(INCLUDES) -o $@ $<

edu_utokyo_vlex_VlexNative.cpp: target/classes/edu/utokyo/vlex/VlexNative.class
	javah -cp target/classes edu.utokyo.vlex.VlexNative

target/classes/edu/utokyo/vlex/VlexNative.class:
	mvn package

update-java:
	mvn clean && mvn package

run:
	bash ./run.sh --local

test:
	make && make run

clean:
	mvn clean && rm -rf obj/*.o *.class $(TARGET)