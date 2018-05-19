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
#include "../common/epoll/epoll_helpers.h"

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

static struct message *parse_user_input()
{
	struct message *send_msg;
	char *input;
	int bytes;

	input = (char *)calloc(1, MAX_CMDLINE_INPUT);
	if (!input) {
		perror("calloc");
		return NULL;
	}

	bytes = read(STDIN_FILENO, input, MAX_CMDLINE_INPUT);
	if (bytes < 0) {
		perror("read from stdin");
		free(input);
		return NULL;
	}

	send_msg = (struct message *)calloc(1, sizeof(*send_msg));
	if (!send_msg) {
		perror("calloc");
		free(input);
		return NULL;
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
		return NULL;
	}

	free(input);

	return send_msg;
}

int main(int argc, char *argv[])
{
	int sockfd, epollfd;

	/* No reason to continue if we can't connect to the server */
	if (connect_to_server(&sockfd))
		exit(EXIT_FAILURE);

	if (create_epoll_manager(&epollfd))
		goto exit_fail_close_sockfd;

	/* Listen for messages from the server */
	if (add_epoll_member(epollfd, sockfd, SOCKET_EPOLL_NEW_MEMBER))
		goto exit_fail_close_epollfd;

	/* Listen for user input */
	if (add_epoll_member(epollfd, STDIN_FILENO, EPOLLIN))
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

		for_each_epoll_event(i, nfds) {
			uint32_t event_mask = events[i].events;
			int eventfd = events[i].data.fd;

			debug_print_epoll_event(eventfd, event_mask);

			switch (event_mask) {
			case EPOLLIN:
				if (eventfd == STDIN_FILENO) {
					struct message *send_msg;
					int bytes;

					send_msg = parse_user_input();
					if (!send_msg)
						continue;

					bytes = send(sockfd, send_msg, MSG_SIZE, 0);
					if (bytes != MSG_SIZE) {
						perror("Error sending message to server");
						/* TODO: Decide if we should just print the error message and
						 * 		 continue to go on about our business
						 */
						free(send_msg);
						goto exit_fail_close_epollfd;
					}
					free(send_msg);

				} else if (eventfd == sockfd) { /* Received CHAT */
					struct message *recv_msg;

					recv_msg = (struct message *)calloc(1, sizeof(*recv_msg));
					if (!recv_msg) {
						perror("calloc");
						goto exit_fail_close_epollfd;
					}

					bytes = recv(sockfd, recv_msg, MSG_SIZE, MSG_WAITALL);
					if (bytes < 0) {
						perror("recv");
						goto exit_fail_close_epollfd;
					}

					printf("(%s) %s: %s\n", recv_msg->chat.channel_name,
					       recv_msg->chat.src_user, recv_msg->chat.text);

				}

				break;

			/* FIXME: Have client try to reconnect on server
			 * disconnect a few times atleast.
			 */
			case (EPOLLIN | EPOLLRDHUP):
				if (eventfd == sockfd) {
					struct message *recv_msg = (struct message *)
						calloc(1, sizeof(*recv_msg));
					if (!recv_msg) {
						perror("calloc");
						goto exit_fail_close_epollfd;
					}

					if (recv(sockfd, recv_msg, MSG_SIZE, MSG_WAITALL) == 0) {
						free(recv_msg);
						goto exit_success;
					}
				}

				break;

			default:
				printf("epoll event %d from fd %d not supported!\n",
				       event_mask, eventfd);
				break;
			}
		}
	}

exit_success:
	close(sockfd);
	close(epollfd);

	return EXIT_SUCCESS;

exit_fail_close_epollfd:
	close(epollfd);

exit_fail_close_sockfd:
	close(sockfd);

	return EXIT_FAILURE;
}

