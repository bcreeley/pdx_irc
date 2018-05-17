/**
 * client.c - Main file for the pdx irc server
 * Author: Brett Creeley
 */

#include "../common/protocol.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>

#define MAX_EPOLL_EVENTS	10
#define MAX_CMDLINE_INPUT	1024
/**
 * connect_to_server - attempt to connect to the irc server
 * @sockfd: pointer to the socket file descriptor that is populated on success
 *
 * On success returns 0, otherwise return -1
 */
int connect_to_server(int *sockfd)
{
	struct sockaddr_in serv_addr = { 0 };
	int result;

	if (!sockfd)
		return -1;

	/* Want to allow receiving/sending on demand */
	*sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (*sockfd == -1) {
		perror("Error creating socket");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
		perror("Error connecting to server socket");
		return -1;
	}

	return 0;
}

static int add_epoll_fd(int epollfd, int addfd, uint32_t events)
{
	struct epoll_event epoll_ev;

	epoll_ev.events = events;
	epoll_ev.data.fd = addfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, addfd, &epoll_ev) == -1) {
		perror("epoll_ctl: addfd");
		return -1;
	}

	return 0;
}

static int rm_epoll_fd(int epollfd, int rmfd)
{
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, rmfd, NULL) == -1) {
		perror("epoll_ctl: rmfd");
		return -1;
	}
	close(rmfd);

	return 0;
}


static int setup_epoll_fd(int *epollfd, int serverfd /* this 2nd argument doesn't belong here */)
{
	if (!epollfd)
		return -1;

	*epollfd = epoll_create1(0);
	if (*epollfd < 0) {
		perror("epoll_create1");
		return -1;
	}

	if (add_epoll_fd(*epollfd, serverfd, EPOLLIN | EPOLLRDHUP)) {
		printf("add_epoll_fd failed!\n");
		close(*epollfd);
		return -1;
	}

	return 0;
}


int main(int argc, char *argv[])
{
	struct message *send_msg, *recv_msg;
	int sockfd, epollfd;
	char *input = NULL;

	recv_msg = (struct message *)malloc(sizeof(*recv_msg));
	if (!recv_msg) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	send_msg = (struct message *)malloc(sizeof(*send_msg));
	if (!send_msg) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	/* No reason to continue if we can't connect to the server */
	if (connect_to_server(&sockfd))
		goto exit_free_msg_mem;

	if (setup_epoll_fd(&epollfd, sockfd))
		goto exit_fail_close_sockfd;

	if (add_epoll_fd(epollfd, STDIN_FILENO, EPOLLIN))
		goto exit_fail_close_epollfd;

	while (1) {
		struct epoll_event events[MAX_EPOLL_EVENTS];
		int nfds, eventfd, i, bytes;

		dprintf(STDOUT_FILENO, "pdx_irc> ");

		nfds = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			goto exit_fail_close_epollfd;
		}

		for (i = 0; i < nfds; ++i) {
			uint32_t event_mask = events[i].events;
			int eventfd = events[i].data.fd;

			printf("fd=%d; events:%s%s%s%s%s%s%s\n",
			       eventfd,
			       (event_mask & EPOLLIN)      ? " EPOLLIN"      : "",
			       (event_mask & EPOLLPRI)     ? " EPOLLPRI"     : "",
			       (event_mask & EPOLLRDHUP)   ? " EPOLLRDHUP"   : "",
			       (event_mask & EPOLLOUT)     ? " EPOLLOUT"     : "",
			       (event_mask & EPOLLET)      ? " EPOLLET"      : "",
			       (event_mask & EPOLLONESHOT) ? " EPOLLONESHOT" : "",
			       (event_mask & EPOLLERR)     ? " EPOLLERR"     : "",
			       (event_mask & EPOLLHUP)     ? " EPOLLHUP"     : "");

			switch (event_mask) {
			case EPOLLIN:
				if (eventfd == STDIN_FILENO) {
					input = (char *)calloc(1, MAX_CMDLINE_INPUT);
					if (!input) {
						perror("calloc");
						goto exit_fail_close_epollfd;
					}

					bytes = read(STDIN_FILENO, input, MAX_CMDLINE_INPUT);
					if (bytes < 0) {
						perror("read from stdin");
						goto exit_fail_read_stdin;
					}

					if (strcasestr(input, ":JOIN") != NULL) {
						send_msg->type = JOIN;
						strncpy(send_msg->join.src_user,
							"Bquigs",
							sizeof("Bquigs"));
						strncpy(send_msg->join.channel_name,
							"LinuxFTW!",
							sizeof("LinuxFTW!"));
					} else if (strcasestr(input, ":CHAT") != NULL) {
						send_msg->type = CHAT;
						strncpy(send_msg->chat.src_user,
							"Bquigs",
							sizeof("Bquigs"));
						strncpy(send_msg->chat.channel_name,
							"LinuxFTW!",
							sizeof("LinuxFTW!"));
						strncpy(send_msg->chat.text,
							"Hello Server!",
							sizeof("Hello Server!"));
					} else {
						printf("Help:\n");
						printf("\t:JOIN  <channel_name>\n");
						printf("\t:LEAVE <channel_name>\n");
						printf("\t:CHAT  <channel_name> <chat message>\n");
						continue;
					}

					free(input);

					bytes = send(sockfd, send_msg, MSG_SIZE, 0);
					if (bytes != MSG_SIZE) {
						perror("Error sending message to server");
						/* TODO: Decide if we should just print the error message and
						 * 		 continue to go on about our business
						 */
						goto exit_free_msg_mem;
					}
				}
				break;

			/* FIXME: Have client try to reconnect on server
			 * disconnect a few times atleast.
			 */
			case (EPOLLIN | EPOLLRDHUP):
				if (eventfd == sockfd)
					if (recv(sockfd, recv_msg, MSG_SIZE, MSG_WAITALL) == 0)
						goto exit_success;
				break;

			default:
				printf("epoll event %d from fd %d not supported!\n",
				       event_mask, eventfd);
				break;
			}
		}
	}

exit_success:
	free(send_msg);
	free(recv_msg);
	close(sockfd);
	close(epollfd);

	return EXIT_SUCCESS;

exit_fail_read_stdin:
	free(input);

exit_fail_close_epollfd:
	close(epollfd);

exit_fail_close_sockfd:
	close(sockfd);

exit_free_msg_mem:
	free(send_msg);

	return EXIT_FAILURE;
}

