# Compiler
CXX = g++
CPPFLAGS = -std=c++17 -O3 -flto -msse4.2
# CPPFLAGS = -std=c++17 -O0 -g -flto -msse4.2
# DEBUGFLAGS = -Wall -Wextra
BENCHFLAGS = -DBENCH
INCLUDES = -I../../src
FLAGS = $(CPPFLAGS) $(DEBUGFLAGS) $(BENCHFLAGS) $(INCLUDES)

# Your program
MAIN = main.cpp
MAIN_O = main.exe

# Compile your program
all:
	$(CXX) $(FLAGS) $(MAIN) -o $(MAIN_O)

FILENAME = /home/natsuoiida/research/vlex/example/JSON/yelp_academic_dataset_business.json

test:
	make clean && make && make run

run:
	./$(MAIN_O) $(FILENAME)

clean:
	rm -f $(MAIN_O)
