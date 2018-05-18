# Top level Makefile for both server and client
# Author: Brett Creeley

SUBDIRS=pdx_irc_client pdx_irc_server
INCLUDES=-I common/ common/epoll/ common/list

.PHONY: default
default:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

.PHONY: clean
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done


