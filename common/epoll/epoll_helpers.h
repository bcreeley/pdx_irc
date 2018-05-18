#ifndef _EPOLL_HELPERS_H
#define _EPOLL_HELPERS_H

#define MAX_EPOLL_EVENTS 10

#define SOCKET_EPOLL_DISCONNECT (EPOLLIN | EPOLLRDHUP)
#define SOCKET_EPOLL_NEW_MEMBER (EPOLLIN | EPOLLRDHUP)

#define for_each_epoll_event(i, nfds) \
	for (i = 0; i < nfds; ++i)

int create_epoll_manager(int *epollfd);
int add_epoll_member(int epollfd, int memberfd, uint32_t subscribe_events);
int rm_epoll_member(int epollfd, int memberfd);
int accept_new_epoll_member(int epollfd, int listenfd);
int debug_print_epoll_event(int eventfd, uint32_t event_mask);

#endif /* _EPOLL_HELPERS_H */
