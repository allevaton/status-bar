# Makefile
#

CC = clang

all:
	$(CC) main.c -g

clean:
	rmf *.out *.o
