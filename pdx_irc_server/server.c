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
#include "../common/list/list.h"
#include "../common/debug/debug.h"

#define MAX_EPOLL_EVENTS 10

/* Server wide channel list */
static struct list_node *channel_list_head = NULL;

static struct channel *get_channel(char *channel_name)
{
	struct channel c;

	strncpy(c.name, channel_name, CHANNEL_NAME_MAX_LEN);
	printf("c.name %s channel_name %s\n", c.name, channel_name);

	return get_list_node_data(channel_list_head, &c, is_equal_channels);
}

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

static bool is_user_in_channel(struct channel *c, struct user *u)
{
	if (!c || !u)
		return false;

	return list_contains(c->user_list_head, u, is_equal_users);
}

static uint32_t handle_join_msg(int srcfd, struct message *msg)
{
	struct list_node *add_node;
	struct channel *channel;
	struct user *user;

	printf("Received JOIN request for channel %s from %s\n",
	       msg->join.channel_name, msg->join.src_user);

	channel = get_channel(msg->join.channel_name);
	/* Add channel if it doesn't exist already */
	if (!channel && add_channel(&channel_list_head, msg->join.channel_name))
		return RESP_CANNOT_ADD_CHANNEL;

	user = calloc(1, sizeof(*user));
	if (!user) {
		perror("calloc");
		return RESP_MEMORY_ALLOC;
	}

	strncpy(user->name, msg->join.src_user, USER_NAME_MAX_LEN);
	user->fd = srcfd;
	if (is_user_in_channel(channel, user)) {
		printf("User %s already in channel %s\n", user->name,
		       channel->name);
		free(user);
		return RESP_ALREADY_IN_CHANNEL;
	}

	add_node = calloc(1, sizeof(*add_node));
	if (!add_node) {
		perror("calloc");
		free(user);
		return RESP_MEMORY_ALLOC;
	}

	add_node->data = user;

	channel = get_channel(msg->join.channel_name);
	if (!channel) {
		free(user);
		free(add_node);
		return RESP_INVALID_CHANNEL_NAME;
	}

	if (add_list_node(&channel->user_list_head, add_node)) {
		printf("Failed to add user node\n");
		free(user);
		free(add_node);
		return RESP_CANNOT_ADD_USER_TO_CHANNEL;
	} else {
		printf("successfully added user %s with fd %d to channel %s\n",
		      user->name, user->fd, msg->join.channel_name);
	}

	return RESP_SUCCESS;
}

static uint32_t handle_chat_msg(int srcfd, struct message *msg)
{
	struct channel *channel;
	struct list_node *tmp;
	struct user user;
	int bytes;

	/* Print where message came from for testing */
	printf("(%s) %s: %s\n", msg->chat.channel_name, msg->chat.src_user,
	       msg->chat.text);

	/* Setup src_user data to avoid echoing message back to sender */
	strncpy(user.name, msg->chat.src_user, USER_NAME_MAX_LEN);
	user.fd = srcfd;
	/* Make sure this message is directed towards a real channel */
	channel = get_channel(msg->chat.channel_name);
	if (!channel)
		return RESP_INVALID_CHANNEL_NAME;

	if (!is_user_in_channel(channel, &user))
		return RESP_NOT_IN_CHANNEL;

	/* Send chat message to all users in the channel */
	for (tmp = channel->user_list_head; tmp != NULL; tmp = tmp->next) {
		/* Don't echo the chat message back to the sender */
		if (is_equal_users(&user, tmp->data))
			continue;

		bytes = send(((struct user *)(tmp->data))->fd, msg, MSG_SIZE, 0);
		if (bytes != MSG_SIZE) {
			perror("send");
			printf("Failed to send chat message to fd %d\n",
			       ((struct user *)(tmp->data))->fd);
		}
	}

	return RESP_SUCCESS;
}

static uint32_t rm_user_from_channel(struct channel *channel, struct user *user)
{
	struct list_node *the_user;

	if (!channel || !user)
		return -1;

	the_user = rm_list_node(&channel->user_list_head, user, is_equal_users);
	if (!the_user)
		return RESP_NOT_IN_CHANNEL;

	/* Have to delete the user that we removed from the list */
	free(the_user);

	return RESP_SUCCESS;
}

static uint32_t handle_leave_msg(int srcfd, struct message *msg)
{
	struct channel *channel;
	struct user user;

	strncpy(user.name, msg->leave.src_user, USER_NAME_MAX_LEN);
	user.fd = srcfd;

	channel = get_channel(msg->leave.channel_name);
	if (!channel)
		return RESP_INVALID_CHANNEL_NAME;

	return rm_user_from_channel(channel, &user);
}

static void build_response_msg(struct message *send_msg, struct message *recv_msg)
{
	send_msg->type = recv_msg->type;

	switch (recv_msg->type) {
	case JOIN:
		strncpy(send_msg->join.src_user, recv_msg->join.src_user,
			USER_NAME_MAX_LEN);
		strncpy(send_msg->join.channel_name,
			recv_msg->join.channel_name, CHANNEL_NAME_MAX_LEN);
		break;
	case LEAVE:
		strncpy(send_msg->leave.src_user, recv_msg->leave.src_user,
			USER_NAME_MAX_LEN);
		strncpy(send_msg->leave.channel_name, recv_msg->leave.channel_name,
			CHANNEL_NAME_MAX_LEN);
		break;
	case CHAT:
		strncpy(send_msg->chat.src_user, recv_msg->chat.src_user,
			USER_NAME_MAX_LEN);
		strncpy(send_msg->chat.channel_name, recv_msg->chat.channel_name,
			CHANNEL_NAME_MAX_LEN);
		strncpy(send_msg->chat.text, recv_msg->chat.text,
			CHAT_MSG_MAX_LEN);
		break;
	default:
		printf("Invalid/unimplemented message type %s\n",
		       msg_type_to_str(recv_msg->type));
		break;
	}
}

#if 0
/* TODO: Implement this to send list of channels to user */
static int handle_list_channels_msg(int srcfd, struct message *msg)
{
	for_each_list_node(channel_list_head) {

	}

	return 0;
}
#endif

static void handle_recv_msg(int epollfd, int srcfd)
{
	struct message *recv_msg, *send_msg;
	int bytes;

	recv_msg = (struct message *)calloc(1, sizeof(*recv_msg));
	if (!recv_msg) {
		perror("calloc");
		return;
	}

	send_msg = (struct message *)calloc(1, sizeof(*send_msg));
	if (!send_msg) {
		perror("calloc");
		free(recv_msg);
		return;
	}

	bytes = recv(srcfd, recv_msg, MSG_SIZE, MSG_WAITALL);
	if (bytes != MSG_SIZE) {
		send_msg->type = ERROR;
		send_msg->response = RESP_RECV_MSG_FAILED;
		printf("%s:%d ERROR!\n", __func__, __LINE__);
		goto send_response;
	}

	switch (recv_msg->type) {
		case JOIN:
			send_msg->response = handle_join_msg(srcfd, recv_msg);
			break;
		case LEAVE:
			printf("User: %s wants to leave channel: %s\n",
			       recv_msg->leave.src_user,
			       recv_msg->leave.channel_name);
			send_msg->response = handle_leave_msg(srcfd, recv_msg);
			break;
		case CHAT:
			send_msg->response = handle_chat_msg(srcfd, recv_msg);
			break;
		default:
			/* Invalid or unimplemented message types */
			printf("Invalid/unimplemented message type %s\n",
			       msg_type_to_str(recv_msg->type));
			break;
	}

	build_response_msg(send_msg, recv_msg);

send_response:
	bytes = send(srcfd, send_msg, MSG_SIZE, 0);
	if (bytes != MSG_SIZE)
		perror("send");

	free(recv_msg);
	free(send_msg);
}

int main(int argc, char *argv[])
{
#define EPOLL_CLIENT_DISCONNECT (EPOLLRDHUP | EPOLLIN)
	struct epoll_event events[MAX_EPOLL_EVENTS];
	int serverfd, epollfd;

	if (setup_server_socket(&serverfd) < 0)
		exit(EXIT_FAILURE);

	if (create_epoll_manager(&epollfd))
		exit(EXIT_FAILURE);

	if (add_epoll_member(epollfd, serverfd, EPOLLIN))
		exit(EXIT_FAILURE);

	printf("serverfd %d\n", serverfd);
	printf("epollfd %d\n", epollfd);

	while (1) {
		int nfds, i;

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
