CC=gcc
CFLAGS=-g -Wall -O -I.
LDFLAGS=
LDLIBS=-lrt

all: main

main: main.o fsm.o 

clean:
	$(RM) *.o *~ main
