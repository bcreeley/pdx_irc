# Makefile for pdx irc client
# Author: Brett Creeley

default: client
.PHONY: default

client: client.o
	gcc -g -o client client.o

client.o: client.c
	gcc -D_GNU_SOURCE -g -c client.c

.PHONY: clean
clean:
	rm -f client *.o

