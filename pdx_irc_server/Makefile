# Makefile for pdx irc server
# Author: Brett Creeley

default: server
.PHONY: default

server: server.o
	gcc -g -o server server.o

server.o: server.c
	gcc -D_GNU_SOURCE -g -c server.c

.PHONY: clean
clean:
	rm -f server *.o

