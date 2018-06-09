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
static struct list_node *channel_list_head = NULL;
static bool list_channels_active = false;

/* User can only request for user list once before this list is deleted */
static struct list_node *user_list_head = NULL;
static bool list_users_active = false;

#define MAX_EPOLL_EVENTS	10
#define MAX_CMDLINE_INPUT	1024

#define MIN(a,b) (a < b ? a : b)

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

static void print_usage()
{
	printf("\nAvailable Commands:\n"
	       "\t#JOIN  /<username> /<channel_name>\n"
	       "\t#LEAVE /<username> /<channel_name>\n"
	       "\t#CHAT  /<username> /<channel_name> /<chat_message>\n"
	       "\t#LIST_CHANNELS /<username>\n"
	       "\t#LIST_USERS /<username> /<channel_name>"
	       "\nMaximum Lengths:\n"
	       "\tusername: %d characters\n"
	       "\tchannel_name: %d characters\n"
	       "\tchat_message: %d characters\n\n", USER_NAME_MAX_LEN-1,
	       CHANNEL_NAME_MAX_LEN-1, CHAT_MSG_MAX_LEN-1);
}


static struct message *join_input(char *input)
{
	struct message *msg;
	char *prev, *next;
	int len;

	msg = calloc(1, sizeof(*msg));
	if (!msg) {
		perror("calloc");
		return NULL;
	}

	/* Find the start of username */
	prev = strstr(input, "/");
	if (!prev)
		goto free_msg;
	/* Don't want "/" as part of the username */
	++prev;

	/* Find the start of channel_name */
	next = strstr(prev, "/");
	if (!next)
		goto free_msg;

	len = MIN(USER_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->join.src_user, prev, len);
	msg->join.src_user[len] = '\0';

	/* Don't want "/" as part of the channel_name */
	prev = ++next;
	/* Find the end of channel_name */
	next = strstr(prev, "\n");
	if (!next)
		goto free_msg;

	len = MIN(CHANNEL_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->join.channel_name, prev, len);
	msg->join.channel_name[len] = '\0';

	msg->type = JOIN;

	printf("src_user: %s channel_name: %s\n", msg->join.src_user, msg->join.channel_name);

	return msg;

free_msg:
	printf("Error parsing %s\n", __FUNCTION__);
	free(msg);
	return NULL;
}

static struct message *leave_input(char *input)
{
	struct message *msg;
	char *prev, *next;
	int len;

	msg = calloc(1, sizeof(*msg));
	if (!msg) {
		perror("calloc");
		return NULL;
	}

	/* Find the start of username */
	prev = strstr(input, "/");
	if (!prev)
		goto free_msg;
	/* Don't want "/" as part of the username */
	++prev;

	/* Find the start of channel_name */
	next = strstr(prev, "/");
	if (!next)
		goto free_msg;

	len = MIN(USER_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->leave.src_user, prev, len);
	msg->leave.src_user[len] = '\0';

	/* Don't want "/" as part of the channel_name */
	prev = ++next;
	/* Find the end of channel_name */
	next = strstr(prev, "\n");
	if (!next)
		goto free_msg;

	len = MIN(CHANNEL_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->leave.channel_name, prev, len);
	msg->leave.channel_name[len] = '\0';

	msg->type = LEAVE;
	printf("src_user: %s channel_name: %s\n", msg->leave.src_user, msg->leave.channel_name);

	return msg;

free_msg:
	printf("Error parsing %s\n", __FUNCTION__);
	free(msg);
	return NULL;
}

static struct message *chat_input(char *input)
{
	struct message *msg;
	char *prev, *next;
	int len;

	msg = calloc(1, sizeof(*msg));
	if (!msg) {
		perror("calloc");
		return NULL;
	}

	/* Find the start of username */
	prev = strstr(input, "/");
	if (!prev)
		goto free_msg;
	/* Don't want "/" as part of the username */
	++prev;

	/* Find the start of channel_name */
	next = strstr(prev, "/");
	if (!next)
		goto free_msg;

	len = MIN(USER_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->chat.src_user, prev, len);
	msg->chat.src_user[len] = '\0';

	/* Don't want "/" as part of the channel_name */
	prev = ++next;
	/* Find the end of channel_name */
	next = strstr(prev, "/");
	if (!next)
		goto free_msg;

	len = MIN(CHANNEL_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->chat.channel_name, prev, len);
	msg->chat.channel_name[len] = '\0';

	/* Don't want "/" as part of the chat text */
	prev = ++next;
	next = strstr(prev, "\n");
	if (!next)
		goto free_msg;

	len = MIN(CHAT_MSG_MAX_LEN-1, next-prev);
	strncpy(msg->chat.text, prev, len);
	msg->chat.text[len] = '\0';

	msg->type = CHAT;
	printf("src_user: %s channel_name: %s chat_msg: (%s)\n", msg->chat.src_user,
	       msg->chat.channel_name, msg->chat.text);

	return msg;

free_msg:
	printf("Error parsing %s\n", __FUNCTION__);
	free(msg);
	return NULL;
}

static struct message *list_channels_input(char *input)
{
	struct message *msg;
	char *prev, *next;
	int len;

	msg = calloc(1, sizeof(*msg));
	if (!msg) {
		perror("calloc");
		return NULL;
	}

	/* Find the start of username */
	prev = strstr(input, "/");
	if (!prev)
		goto free_msg;
	/* Don't want "/" as part of the username */
	++prev;

	next = strstr(prev, "\n");
	if (!next)
		goto free_msg;

	len = MIN(USER_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->list_channels.src_user, prev, len);
	msg->list_channels.src_user[len] = '\0';

	msg->type = LIST_CHANNELS;

	return msg;

free_msg:
	/* Allow another request to LIST_CHANNELS */
	list_channels_active = false;
	printf("Error parsing %s\n", __FUNCTION__);
	free(msg);
	return NULL;
}

static struct message *list_users_input(char *input)
{
	struct message *msg;
	char *prev, *next;
	int len;

	msg = calloc(1, sizeof(*msg));
	if (!msg) {
		perror("calloc");
		return NULL;
	}

	/* Find the start of src_usr */
	prev = strstr(input, "/");
	if (!prev)
		goto free_msg;
	/* Don't want "/" as part of the username */
	++prev;

	next = strstr(prev, "/");
	if (!next)
		goto free_msg;

	len = MIN(USER_NAME_MAX_LEN-1, next-prev);
	strncpy(msg->list_users.src_user, prev, len);
	msg->list_users.src_user[len] = '\0';

	/* Don't want "/" as part of the chat text */
	prev = ++next;
	next = strstr(prev, "\n");
	if (!next)
		goto free_msg;

	len = MIN(CHAT_MSG_MAX_LEN-1, next-prev);
	strncpy(msg->list_users.channel_name, prev, len);
	msg->list_users.channel_name[len] = '\0';

	msg->type = LIST_USERS;

	return msg;

free_msg:
	/* Allow another request to LIST_CHANNELS */
	list_users_active = false;
	printf("Error parsing %s\n", __FUNCTION__);
	free(msg);
	return NULL;
}

static struct message *parse_user_input()
{
	struct message *send_msg = NULL;
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

	if (strcasestr(input, "help"))
		print_usage();
	else if (strcasestr(input, "#JOIN"))
		send_msg = join_input(input);
	else if (strcasestr(input, "#LEAVE"))
		send_msg = leave_input(input);
	else if (strcasestr(input, "#CHAT"))
		send_msg = chat_input(input);
	else if (strcasestr(input, "#LIST_CHANNELS") && !list_channels_active)
		send_msg = list_channels_input(input);
	else if (strcasestr(input, "#LIST_USERS") && !list_users_active)
		send_msg = list_users_input(input);
	else
		printf("Unsupported message type");

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
	if (bytes != MSG_SIZE) {
		perror("recv");
		ret = -1;
		goto out;
	}

	switch (recv_msg->type) {
	case CHAT:
		if (recv_msg->response == RESP_SUCCESS)
			printf("(%s) %s: %s\n", recv_msg->chat.channel_name,
		      	       recv_msg->chat.src_user, recv_msg->chat.text);

		break;
	case LIST_CHANNELS:
		if (recv_msg->response & RESP_LIST_CHANNELS_IN_PROGRESS) {
			printf("adding channel %s to list\n", recv_msg->list_channels.channel_name);
			ret = add_channel(&channel_list_head,
					  recv_msg->list_channels.channel_name);
			if (ret)
				printf("Failed to add channel!\n");

		} else if (recv_msg->response & RESP_DONE_SENDING_CHANNELS) {
			printf("Channel List:\n");
			print_channel_list(channel_list_head);
			del_channel_list(&channel_list_head);
			/* Allow another request to LIST_CHANNELS */
			list_channels_active = false;
		} else {
			printf("Invalid response %s from server\n",
			       resp_type_to_str(recv_msg->response));
		}


		break;
	case LIST_USERS:
		if (recv_msg->response & RESP_LIST_USERS_IN_PROGRESS) {
			ret = add_user(&user_list_head,
				       recv_msg->list_users.username);
			if (ret)
				printf("Failed to add user!\n");

		} else if (recv_msg->response & RESP_DONE_SENDING_USERS) {
			printf("User List for channel: %s\n",
			       recv_msg->list_users.channel_name);
			print_user_list(user_list_head);
			del_user_list(&user_list_head);
			/* Allow another request to LIST_CHANNELS */
			list_users_active = false;
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
					if (!send_msg ||
					    send_msg->type == MSG_TYPE_INVALID)
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

