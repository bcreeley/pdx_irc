#include <sys/epoll.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "epoll_helpers.h"

int add_epoll_member(int epollfd, int memberfd, uint32_t subscribe_events)
{
	struct epoll_event epoll_ev;

	epoll_ev.events = subscribe_events;
	epoll_ev.data.fd = memberfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, memberfd, &epoll_ev) == -1) {
		perror("epoll_ctl: addfd");
		return -1;
	}

	return 0;
}

int rm_epoll_member(int epollfd, int memberfd)
{
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, memberfd, NULL) == -1) {
		perror("epoll_ctl: rmfd");
		return -1;
	}
	close(memberfd);

	return 0;
}


int create_epoll_manager(int *epollfd)
{
	if (!epollfd)
		return -1;

	*epollfd = epoll_create1(0);
	if (*epollfd < 0) {
		perror("epoll_create1");
		return -1;
	}

	return 0;
}

int accept_new_epoll_member(int epollfd, int listenfd)
{
	int clientfd;

	clientfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
	if (clientfd == -1) {
		perror("accept");
		return -1;
	}

	if (add_epoll_member(epollfd, clientfd, SOCKET_EPOLL_NEW_MEMBER))
		return -1;

	return 0;
}

void debug_print_epoll_event(int eventfd, uint32_t event_mask)
{
	printf("fd=%d; events:%s%s%s%s%s%s%s%s\n",
	       eventfd,
	       (event_mask & EPOLLIN)      ? " EPOLLIN"      : "",
	       (event_mask & EPOLLPRI)     ? " EPOLLPRI"     : "",
	       (event_mask & EPOLLRDHUP)   ? " EPOLLRDHUP"   : "",
	       (event_mask & EPOLLOUT)     ? " EPOLLOUT"     : "",
	       (event_mask & EPOLLET)      ? " EPOLLET"      : "",
	       (event_mask & EPOLLONESHOT) ? " EPOLLONESHOT" : "",
	       (event_mask & EPOLLERR)     ? " EPOLLERR"     : "",
	       (event_mask & EPOLLHUP)     ? " EPOLLHUP"     : "");
}

