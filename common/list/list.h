#ifndef _LIST_H
#define _LIST_H

#include "../protocol.h"
#include <stdbool.h>
#include <string.h>

struct channel {
	char name[CHANNEL_NAME_MAX_LEN];
	char num_users;
	struct user_list_node *user_list_head;
};

bool is_equal_channels(void *c1, void *c2)
{
	char *c1_name, *c2_name;

	if (!c1 || !c2)
		return false;

	c1_name = ((struct channel *)c1)->name;
	c2_name = ((struct channel *)c2)->name;
	if (strcmp(c1_name, c2_name) == 0)
		return true;

	return false;
}

struct user {
	char name[USER_NAME_MAX_LEN];
	int fd;
};

bool is_equal_users(void *u1, void *u2)
{
	if (!u1 || !u2)
		return false;

	return (((struct user *)u1)->fd == ((struct user *)u2)->fd);
}

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

#endif /* _LIST_H */
