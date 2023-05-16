CC = g++
CFLAGS = -std=c++17 -fPIC -mavx512f -mavx512bw -mavx512vl -mavx512dq

EXECFLAGS = -Wall -O3

BIN = bin

CONVERT = $(BIN)/convert.exe
CSV = $(BIN)/csv.exe
RJSON = $(BIN)/rjson.exe

all: $(CONVERT) $(CSV) $(RJSON)

$(CONVERT): convert.cpp
	$(CC) $(CFLAGS) $(EXECFLAGS) -o $@ $<

$(CSV): csv.cpp
	$(CC) $(CFLAGS) $(EXECFLAGS) -o $@ $<

$(RJSON): rjson.cpp
	$(CC) $(CFLAGS) $(EXECFLAGS) -o $@ $<