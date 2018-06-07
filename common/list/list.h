#ifndef _LIST_H
#define _LIST_H

#include "../protocol.h"
#include <stdbool.h>
#include <string.h>

#define for_each_list_node(h) \
	struct list_node *tmp; \
	for (tmp = head; tmp != NULL; tmp = tmp->next)

struct channel {
	char name[CHANNEL_NAME_MAX_LEN];
	int num_users;
	struct list_node *user_list_head;
};

bool is_equal_channels(void *c1, void *c2);

#if 0
struct channel *get_channel(struct list_node *head, char *channel_name);
#endif

struct user {
	char name[USER_NAME_MAX_LEN];
	int fd;
};

bool is_equal_users(void *u1, void *u2);

struct list_node {
	struct list_node *next;
	void *data;
};

int add_list_node(struct list_node **head, struct list_node *add);

struct list_node *
rm_list_node(struct list_node **head, void *data,
	     bool (*is_equal)(void *d1, void *d2));

bool list_contains(struct list_node *head, void *data,
		   bool (*is_equal)(void *d1, void *d2));

void *get_list_node_data(struct list_node *head, void *data,
			 bool (*is_equal)(void *d1, void *d2));

#endif /* _LIST_H */
