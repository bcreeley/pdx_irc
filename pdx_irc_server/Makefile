# Makefile for pdx irc server
# Author: Brett Creeley

COMMON_DIR = ../common
LIST_DIR = $(COMMON_DIR)/list
EPOLL_DIR = $(COMMON_DIR)/epoll

SRC = server.c $(EPOLL_DIR)/epoll_helpers.c $(LIST_DIR)/list.c
OBJS = server.o epoll_helpers.o list.o

.PHONY: default
default: server

server: $(OBJS)
	$(CC) $(CFLAGS) -o server $(OBJS)

$(OBJS): $(SRC)
	$(CC) -D_GNU_SOURCE $(CFLAGS) -c $(SRC)

clean:
	rm -f server *.o
