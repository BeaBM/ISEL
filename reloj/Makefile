CC=gcc
CFLAGS=-g -Wall -O -I. -DNDEBUG
LDFLAGS=
LDLIBS=-lrt -lwiringPi

all: main

main: main.o fsm.o 

clean:
	$(RM) *.o *~ main
