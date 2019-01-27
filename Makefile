
CC=gcc
CFLAGS=-Wall -O3

cpufuzz: main.c
	$(CC) $(CFLAGS) -o cpufuzz main.c

clean:
	rm cpufuzz

