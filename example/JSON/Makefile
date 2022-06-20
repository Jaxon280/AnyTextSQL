# Compiler
CXX = g++
CPPFLAGS = -std=c++17 -O3 -flto -msse4.2
DEBUGFLAGS = -std=c++17 -O0 -g -flto -msse4.2 -Wall -Wextra
BENCHFLAGS = -DBENCH
INCLUDES = -I../../include
FLAGS = $(BENCHFLAGS) $(INCLUDES)

# Your program
CPP_CODES = main.cpp $(wildcard ../../src/*.cpp)
MAIN = main_codegen.cpp
MAIN_CODE = main_codegen.exe
MAIN_O = main.exe
MAIN_ASM = main.asm

# Compile your program
all:
	$(CXX) $(FLAGS) $(CPPFLAGS) $(CPP_CODES) -o $(MAIN_O)

codegen:
	$(CXX) $(FLAGS) $(CPPFLAGS) $(MAIN) -o $(MAIN_CODE)

FILENAME = /home/natsuoiida/research/vlex/example/JSON/yelp_academic_dataset_business.json

test:
	make clean && make && make run


debug:
	$(CXX) $(FLAGS) $(DEBUGFLAGS) $(CPP_CODES) -o $(MAIN_O)

run:
	./$(MAIN_O) $(FILENAME)

asm:
	$(CXX) -S -O3 -msse4.2 $(CPP_CODES) -o $(MAIN_ASM)

clean:
	rm -f $(MAIN_O)
