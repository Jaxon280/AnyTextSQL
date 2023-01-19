# Compiler
CC = g++
CFLAGS = -c -std=c++17 -fPIC -mavx512f -mavx512bw -mavx512vl -mavx512dq

EXECFLAGS = -Wall
EXECFLAGS += -O3 -DBENCH
# EXECFLAGS += -O0 -g -Wextra

LDFLAGS = -shared
# BENCHFLAGS = -DBENCH
INCLUDES = -I../include -I../include/parser/query -I../include/parser/regex -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
TARGET = libvlex.so
ODIR = obj
SDIR = ../src

_OBJS = edu_utokyo_vlex_VlexNative.o charCode.o command.o commandNode.o commandParser.o commandScanner.o dfa.o executor.o ios.o optimizer.o pfa.o queryNode.o queryParser.o queryScanner.o regexNode.o regexParser.o regexScanner.o runtime.o vfa.o
OBJS = $(patsubst %, $(ODIR)/%, $(_OBJS))

all: $(TARGET)

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) $(EXECFLAGS) $(INCLUDES) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(INCLUDES) -o $(TARGET) $(LIBRARIES) $(LDFLAGS) $^

$(ODIR)/edu_utokyo_vlex_VlexNative.o: edu_utokyo_vlex_VlexNative.cpp
	$(CC) $(CFLAGS) $(EXECFLAGS) $(INCLUDES) -o $@ $<

edu_utokyo_vlex_VlexNative.cpp: target/classes/edu/utokyo/vlex/VlexNative.class
	javah -cp target/classes edu.utokyo.vlex.VlexNative

target/classes/edu/utokyo/vlex/VlexNative.class:
	mvn package

run:
	bash ./run.sh --local

test:
	make && make run

clean:
	mvn clean && rm -rf obj/*.o *.class $(TARGET)