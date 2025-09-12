# Simple Makefile for server.cpp

all:
	g++ -std=c++17 server.cpp -o server

run: all
	./server

clean:
	rm -f server