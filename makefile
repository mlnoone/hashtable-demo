CC=gcc
CFLAGS=-I.
LIBS=-lm

demo: demo.c hashtable.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

