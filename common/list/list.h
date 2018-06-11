#ifndef _LIST_H
#define _LIST_H

#include "../protocol.h"
#include <stdbool.h>
#include <string.h>

struct channel {
	char name[CHANNEL_NAME_MAX_LEN];
	int num_users;
	struct list_node *user_list_head;
};

struct user {
	char name[USER_NAME_MAX_LEN];
	int fd;
};

void print_channel(void *d);
void print_channel_list(struct list_node *head);

void del_user_list(struct list_node **head);
void del_user_data(void **u);
void del_channel_list(struct list_node **head);
void del_channel_data(void **c);

void print_list(struct list_node *head, void (*print_data)(void *d));
void del_list(struct list_node  **head, void (*del_data)(void **d));

int add_channel(struct list_node **head, char *channel_name);
bool is_equal_channels(void *c1, void *c2);

void print_user(void *d);
void print_user_list(struct list_node *head, char *channel_name);
int add_user(struct list_node **head, char *username);
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
