# Makefile
#

CC = clang
CFLAGS =

all:
	$(CC) main.c $(CFLAGS)

clean:
	rmf *.out *.o
