# CC = Your compiler
CC=g++

# CFLAGS = CompilatorFLAGS
CFLAGS=-c

all: asm cpu my_make

asm: asm.o
	$(CC) asm.o -o asm

asm.o: asm.cpp
	$(CC) $(CFLAGS) asm.cpp

cpu: cpu.o stack.o
	$(CC) cpu.o stack.o -o cpu

my_make: make.o
	$(CC) make.o -o my_make

cpu.o: cpu.cpp
	$(CC) $(CFLAGS) cpu.cpp

stack.o: stack/stack.cpp
	$(CC) $(CFLAGS) stack/stack.cpp

make.o: make.cpp
	$(CC) $(CFLAGS) make.cpp

clean:
	rm -rf *.o cpu asm my_make
