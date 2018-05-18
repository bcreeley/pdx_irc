# Makefile for pdx irc client
# Author: Brett Creeley

CFLAGS+=-g

COMMON_DIR = ../common
LIST_DIR = $(COMMON_DIR)/list
EPOLL_DIR = $(COMMON_DIR)/epoll

SRC = client.c $(EPOLL_DIR)/epoll_helpers.c $(LIST_DIR)/list.c
OBJS = client.o epoll_helpers.o list.o

.PHONY: client
client: $(OBJS)
	$(CC) $(CFLAGS) -o client $(OBJS)

$(OBJS): $(SRC)
	$(CC) -D_GNU_SOURCE $(CFLAGS) -c $(SRC)

clean:
	rm -f client *.o
