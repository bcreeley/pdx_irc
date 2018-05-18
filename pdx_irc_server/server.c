/**
 * server.c - Main file for the pdx irc server
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

#define MAX_EPOLL_EVENTS 10

int setup_server_socket(int *serverfd)
{
	struct sockaddr_in serv_addr = { 0 };
	int yes = 1;

	if (!serverfd)
		return -1;

	*serverfd = socket(PF_INET, SOCK_STREAM, 0);
	if (*serverfd < 0) {
		perror("Error opening socket\n");
		return -1;
	}

	if (setsockopt(*serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))
	    == -1) {
		perror("setsockopt");
		goto err_closefd;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000);

	if (bind(*serverfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))
	    != 0) {
		perror("Error binding socket\n");
		goto err_closefd;
	}

	if (listen(*serverfd, 10) != 0) {
		perror("Error listening on socket\n");
		goto err_closefd;
	}

	return 0;

err_closefd:
	close(*serverfd);
	return -1;
}

static void handle_recv_msg(int epollfd, int srcfd)
{
	struct message *recv_msg;
	int bytes;
	int i;

	recv_msg = (struct message *)calloc(1, sizeof(*recv_msg));
	if (!recv_msg) {
		perror("calloc");
		return;
	}

	bytes = recv(srcfd, recv_msg, MSG_SIZE, MSG_WAITALL);
	/* This means the client disconnected from us */
	if (!bytes) {
		printf("read 0 bytes!\n");
		goto free_recv_msg;
	} else if (bytes != MSG_SIZE) {
		printf("%s:%d ERROR!\n", __func__, __LINE__);
	}

	switch (recv_msg->type) {
		case JOIN:
			printf("Received JOIN request for channel %s from %s\n",
			       recv_msg->join.channel_name,
			       recv_msg->join.src_user);
			//send_msg.type = JOIN;
			//send_msg.join_response.result = SUCCESS;
			break;
		case LEAVE:
			printf("User: %s wants to leave channel: %s\n",
			       recv_msg->leave.src_user,
			       recv_msg->leave.channel_name);
			/* TODO: Remove user from specific channel's user list */
			break;
		case CHAT:
			/* Print where message came from for testing */
			printf("(%s) %s: %s\n", recv_msg->chat.channel_name,
			       recv_msg->chat.src_user, recv_msg->chat.text);
#if 0
			bytes = send(6, recv_msg, MSG_SIZE, 0);
			if (bytes != MSG_SIZE) {
				perror("send");
			}
#endif
			/* Get ready to relay message to all users */
			//	send_msg.type = CHAT;
			//	strncpy(send_msg.chat.text, recv_msg->chat.text,
			//			CHAT_MSG_MAX_LEN);
			//	strncpy(send_msg.chat.src_user, recv_msg->chat.channel_name,
			//			CHANNEL_NAME_MAX_LEN);
			//	printf("Relaying chat message:\n\t\"%s\" to all users\n",
			//	       send_msg.chat.text);
			/* TODO: Find a way to send this message to all users that are part
			 * of the channel's list that was specified in the recv()'d CHAT
			 * message (i.e. function that loops over the channel's user list
			 * and sends it to all of them except the source user).
			 */
			break;
		default:
			/* Invalid or unimplemented message types */
			break;
	}

free_recv_msg:
	free(recv_msg);
}

int main(int argc, char *argv[])
{
#define EPOLL_CLIENT_DISCONNECT (EPOLLRDHUP | EPOLLIN)
	struct epoll_event epoll_ev, events[MAX_EPOLL_EVENTS];
	int serverfd, connfd, epollfd, result;

	if (setup_server_socket(&serverfd) < 0)
		exit(EXIT_FAILURE);

	if (create_epoll_manager(&epollfd))
		exit(EXIT_FAILURE);

	if (add_epoll_member(epollfd, serverfd, EPOLLIN))
		exit(EXIT_FAILURE);

	printf("serverfd %d\n", serverfd);
	printf("epollfd %d\n", epollfd);

	while (1) {
		int nfds, connfd, i;

		nfds = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_SUCCESS);
		}

		for_each_epoll_event(i, nfds) {
			uint32_t event_mask = events[i].events;
			int eventfd = events[i].data.fd;

			debug_print_epoll_event(eventfd, event_mask);

			switch (event_mask) {
			case EPOLLIN:
				/* Only a new client if this is the serverfd */
				if (eventfd == serverfd) {
					if (accept_new_epoll_member(epollfd, eventfd)) {
						printf("accept_new_client() failed!\n");
						exit(EXIT_FAILURE);
					}
					printf("new client on fd %d\n", eventfd);
				} else {
					printf("new message from client on fd %d\n",
					       eventfd);

					handle_recv_msg(epollfd, eventfd);
				}
				break;

			case EPOLL_CLIENT_DISCONNECT:
				if (rm_epoll_member(epollfd, eventfd))
					exit(EXIT_FAILURE);
				printf("removing client fd %d\n",
				       eventfd);
				break;

			case EPOLLERR:
				printf("EPOLLERR on fd %d\n", eventfd);
				close(eventfd);
				break;

			case EPOLLHUP:
				printf("EPOLLHUP on fd %d\n", eventfd);
				close(eventfd);
				break;

			default:
				printf("epoll event %d from fd %d not supported !\n",
				       event_mask, eventfd);
				break;
			}
		}
	}

	return 0;
}
