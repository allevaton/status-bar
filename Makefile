<<<<<<< HEAD
# Makefile
#

CC = clang
CFLAGS =

all:
	$(CC) main.c $(CFLAGS)

clean:
	rmf *.out *.o
||||||| merged common ancestors
=======
# Makefile
#

CC = clang

all:
	$(CC) main.c -g

clean:
	rmf *.out *.o
>>>>>>> 078aa37d1ef9c2c00f9dddfe5f200f3fbb7b978f
