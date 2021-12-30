# Compiler
CXX = g++
# CPPFLAGS = -std=c++17 -O3 -flto -msse4.2
CPPFLAGS = -std=c++17 -O3 -flto
# CPPFLAGS = -std=c++17 -O0 -g -flto -msse4.2
# DEBUGFLAGS = -Wall -Wextra
# BENCHFLAGS = -DBENCH
FLAGS = $(CPPFLAGS) $(DEBUGFLAGS) $(BENCHFLAGS)

# Your program
MAIN = main.cpp
MAIN_O = main.exe

# Compile your program
all:
	$(CXX) $(FLAGS) $(MAIN) -o $(MAIN_O)

test:
	make clean && make && make run

run:
	./$(MAIN_O)

clean:
	rm -f $(MAIN_O)
