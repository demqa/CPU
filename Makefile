# CC = Your compiler
CC=g++

# CFLAGS = CompilatorFLAGS
CFLAGS=-c

all: cpu

cpu: cpu.o stack.o
	$(CC) cpu.o stack.o -o cpu

cpu.o: cpu.cpp
	$(CC) $(CFLAGS) cpu.cpp

stack.o: stack/stack.cpp
	$(CC) $(CFLAGS) stack/stack.cpp

clean:
	rm -rf *.o cpu