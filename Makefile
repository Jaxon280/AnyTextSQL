# Compiler
CXX = g++
# CPPFLAGS = -std=c++17 -O3 -flto -msse4.2
CPPFLAGS = -std=c++17 -O3 -flto
# CPPFLAGS = -std=c++17 -O0 -g -flto -msse4.2
# DEBUGFLAGS = -Wall -Wextra
BENCHFLAGS = -DBENCH
INCLUDES = -I./include/
FLAGS = $(CPPFLAGS) $(DEBUGFLAGS) $(BENCHFLAGS) $(INCLUDES)

# Your program
CPP_CODES = main.cpp $(wildcard src/*.cpp)
MAIN_O = main.exe

FILENAME = /home/natsuoiida/research/vlex/example/JSON/yelp_academic_dataset_business.json

# Compile your program
all:
	$(CXX) $(FLAGS) $(CPP_CODES) -o $(MAIN_O)

test:
	make clean && make && make run

run:
	./$(MAIN_O) $(FILENAME)

clean:
	rm -f $(MAIN_O)
