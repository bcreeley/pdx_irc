#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int add_channel(struct list_node **head, char *channel_name)
{
		struct list_node *add_node;
		struct channel *c;

		if (!channel_name)
			return -1;

		c = calloc(1, sizeof(*c));
		if (!c) {
			perror("calloc");
			return -1;
		}

		add_node = calloc(1, sizeof(*add_node));
		if (!add_node) {
			perror("malloc");
			free(c);
			return -1;
		}

		strncpy(c->name, channel_name, CHANNEL_NAME_MAX_LEN);
		add_node->data = c;
		if (add_list_node(head, add_node)) {
			printf("Failed to add channel node\n");
			free(c);
			free(add_node);
			return -1;
	}

	return 0;
}

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

#if 0
struct channel *get_channel(struct list_node *head, char *channel_name)
{
	struct channel c;

	strncpy(c.name, channel_name, CHANNEL_NAME_MAX_LEN);

	return get_list_node_data(head, &c, is_equal_channels);
}
#endif

bool is_equal_users(void *u1, void *u2)
{
	struct user *user1, *user2;

	if (!u1 || !u2)
		return false;

	user1 = u1;
	user2 = u2;

	if (user1->fd == user2->fd && strcmp(user1->name, user2->name) == 0)
		return true;

	return false;
}

/**
 * add_list_node - Adds a list_node right after the head of the list
 *
 * @head - head list_node of the list
 * @add - list_node to be added
 *
 * Returns -1 on error, 0 on success
 */
int add_list_node(struct list_node **head, struct list_node *add)
{
	struct list_node *tmp = *head;

	if (!add)
		return -1;

	if (!tmp) {
		add->next = NULL;
		*head = add;
	} else {
		add->next = tmp->next;
		tmp->next = add;
	}

	return 0;
}

/**
 * rm_list_node - removes a list_node from the list and returns it
 *
 * @head - pointer to the head of the list so it can be updated if needed
 * @data - data to find in a list_node of the list pointed to by *head
 * @is_equal - function used for comparing data sent in with list_node data
 *
 * Returns the list_node with data matching the passed in data on success,
 * otherwise returns NULL when a list_node with matching data cannot be found.
 * Note: This function does not free the list_node but returns it after
 * reconnecting the nodes around the found list_node.
 */
struct list_node *rm_list_node(struct list_node **head, void *data,
			       bool (*is_equal)(void *d1, void *d2))
{
	struct list_node *tmp = *head;

	if (!tmp || !data)
		return NULL;

	/* If the head of the list matches the data sent in */
	if (is_equal(tmp->data, data)) {
	       *head = (*head)->next;
		return tmp;
	}

	while (tmp->next != NULL) {
		struct list_node *prev = tmp;

		tmp = tmp->next;
		if (is_equal(tmp->data, data)) {
			/* Found the node, remove and return it */
			prev->next = tmp->next;
			tmp->next = NULL;
			return tmp;
		}
	}

	/* Could not find data in the list */
	return NULL;
}

/**
 * list_contains - checks if the list has the data sent in as an argument
 *
 * @head: head of the list used to iterate through the list
 * @data: data to look for in the list
 * @is_equal: function used for comparing data sent in with list_node data
 *
 * Returns true if the data is found in the list, otherwise returns false
 */
bool list_contains(struct list_node *head, void *data,
		   bool (*is_equal)(void *d1, void *d2))
{
	struct list_node *tmp;

	if (!data)
		return false;

	for (tmp = head; tmp != NULL; tmp = tmp->next)
		if (is_equal(tmp->data, data))
			return true;

	return false;
}


/**
 * get_list_node_data - find the data passed in within the list
 *
 * @head: head of the list used to iterate through the list
 * @data: data to look for in the list
 * @is_equal: function used for comparing data sent in with list_node data
 *
 * Returns a pointer to the found data on success, otherwise NULL on failure
 */
void *get_list_node_data(struct list_node *head, void *data,
			 bool (*is_equal)(void *d1, void *d2))
{
	struct list_node *tmp;
	if (!data) {
		printf("list node data is null!\n");
		return NULL;
	}

	for (tmp = head; tmp != NULL; tmp = tmp->next)
		if (is_equal(tmp->data, data))
			return tmp->data;

	return NULL;
}

#if 0
int main(int argc, char *argv[])
{
	struct list_node *head = NULL;

	struct list_node *tmp = NULL;
	struct channel c1, c2, c3;
	struct channel *channel;

	strncpy(c1.name, "LinuxFTW!", sizeof("LINUXFTW!"));
	strncpy(c2.name, "WindowsFTL!", sizeof("WindowsFTL"));
	strncpy(c3.name, "GraduationFTW!", sizeof("GraduationFTW!"));

	tmp = calloc(1, sizeof(struct list_node));
	tmp->data = (void *)&c1;
	if (add_list_node(&head, tmp))
		printf("Failed to add c1 list_node\n");

	tmp = calloc(1, sizeof(struct list_node));
	tmp->data = (void *)&c2;
	if (add_list_node(&head, tmp))
		printf("Failed to add c2 list_node\n");


	channel = get_channel(head, c3.name);
	if (!channel)
		printf("Could not find list_node with c3\n");
	else
		printf("channel->name = %s\n", channel->name);


	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		printf("tmp->data = %s\n", ((struct channel *)(tmp->data))->name);
	}

	if (!list_contains(head, (void *)&c3, is_equal_channels))
		printf("Could not find channel %s!\n", c3.name);

	tmp = rm_list_node(&head, (void *)&c1, is_equal_channels);
	if (tmp != NULL) {
		printf("removed channel c1 %s\n", ((struct channel *)(tmp->data))->name);
		free(tmp);
	}

	tmp = rm_list_node(&head, (void *)&c1, is_equal_channels);
	if (tmp == NULL)
		printf("Could not find channel c1 %s!\n", c1.name);

	tmp = rm_list_node(&head, (void *)&c2, is_equal_channels);
	if (tmp != NULL) {
		printf("removed channel c2 %s\n", ((struct channel *)(tmp->data))->name);
		free(tmp);
	}

	tmp = rm_list_node(&head, (void *)&c2, is_equal_channels);
	if (tmp == NULL)
		printf("Could not find channel c2 %s!\n", c2.name);

#if 0 /* clear list */
	while (head != NULL) {
		tmp = head->next;

		free(head);
		head = tmp;
	}
#endif
	return 0;
}
#endif
