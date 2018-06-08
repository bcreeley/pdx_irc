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
#include "../common/debug/debug.h"
#include "../common/list/list.h"

/* User can only request for channel list once before this list is deleted */
struct list_node *channel_list_head = NULL;

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

	if (strcasestr(input, ":JOINB") != NULL) {
		send_msg->type = JOIN;
		strcpy(send_msg->join.src_user, "Bquigs");
		strcpy(send_msg->join.channel_name, "LinuxFTW!");
	} else if (strcasestr(input, ":JOINA") != NULL) {
		send_msg->type = JOIN;
		strcpy(send_msg->join.src_user, "Ann");
		strcpy(send_msg->join.channel_name, "LinuxFTW!");
	} else if (strcasestr(input, ":JOIN1") != NULL) {
		send_msg->type = JOIN;
		strcpy(send_msg->join.src_user, "Ann");
		strcpy(send_msg->join.channel_name, "Channel1!");
	} else if (strcasestr(input, ":JOIN2") != NULL) {
		send_msg->type = JOIN;
		strcpy(send_msg->join.src_user, "Ann");
		strcpy(send_msg->join.channel_name, "Channel2!");
	} else if (strcasestr(input, ":CHATB") != NULL) {
		send_msg->type = CHAT;
		strcpy(send_msg->chat.src_user, "Bquigs");
		strcpy(send_msg->chat.channel_name, "LinuxFTW!");
		strcpy(send_msg->chat.text, "Hello Ann!");
	} else if (strcasestr(input, ":CHATA") != NULL) {
		send_msg->type = CHAT;
		strcpy(send_msg->chat.src_user, "Ann");
		strcpy(send_msg->chat.channel_name, "LinuxFTW!");
		strcpy(send_msg->chat.text, "Hello Bquigs!");
	} else if (strcasestr(input, ":LEAVEA") != NULL) {
		send_msg->type = LEAVE;
		strcpy(send_msg->leave.src_user, "Ann");
		strcpy(send_msg->leave.channel_name, "LinuxFTW!");
	} else if (strcasestr(input, ":LEAVEA") != NULL) {
		send_msg->type = LEAVE;
		strcpy(send_msg->leave.src_user, "Bquigs");
		strcpy(send_msg->leave.channel_name, "LinuxFTW!");
	} else if (strcasestr(input, ":LIST CHANNELS") != NULL) {
		send_msg->type = LIST_CHANNELS;
		strcpy(send_msg->list_channels.src_user, "Bquigs");
		send_msg->list_channels.list_key = 1;
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

static int handle_recv_msg(int recvfd)
{
	struct message *recv_msg;
	int ret = 0;
	int bytes;

	recv_msg = (struct message *)calloc(1, sizeof(*recv_msg));
	if (!recv_msg) {
		perror("calloc");
		return -1;
	}

	bytes = recv(recvfd, recv_msg, MSG_SIZE, MSG_WAITALL);
	if (bytes < 0) {
		perror("recv");
		ret = -1;
		goto out;
	}

	switch (recv_msg->type) {
	case CHAT:
		printf("recv_msg type %s response %s\n",
		       msg_type_to_str(recv_msg->type),
		       resp_type_to_str(recv_msg->response));

		if (recv_msg->response == RESP_SUCCESS)
			printf("(%s) %s: %s\n", recv_msg->chat.channel_name,
		      	       recv_msg->chat.src_user, recv_msg->chat.text);

		break;
	case LIST_CHANNELS:
		printf("recv_msg type %s response %s\n",
		       msg_type_to_str(recv_msg->type),
		       resp_type_to_str(recv_msg->response));

		if (recv_msg->response & RESP_LIST_CHANNELS_IN_PROGRESS) {
			printf("adding channel %s to list\n", recv_msg->list_channels.channel_name);
			ret = add_channel(&channel_list_head,
					  recv_msg->list_channels.channel_name);
		} else if (recv_msg->response & RESP_DONE_SENDING_CHANNELS) {
			printf("Channel List:\n");
			print_channel_list(channel_list_head);
			del_channel_list(&channel_list_head);
		} else {
			printf("Invalid response %s from server\n",
			       resp_type_to_str(recv_msg->response));
		}


		break;
	default:
		printf("Unhandled receive message %s with response %s\n",
		       msg_type_to_str(recv_msg->type),
		       resp_type_to_str(recv_msg->response));
		ret = -1;
		break;
	}

out:
	free(recv_msg);
	return ret;
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
		int nfds, i;

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
				/* Read user input */
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

				/* Received CHAT or server response message */
				} else if (eventfd == sockfd) {
					if (handle_recv_msg(sockfd))
						printf("Failed to handle_recv_msg");
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

