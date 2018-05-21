#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * add_list_node - Adds a list_node right after the head of the list
 *
 * @head - head list_node of the list
 * @add - list_node to be added
 *
 * Returns -1 on error, 0 on success
 */
int add_list_node(struct list_node *head, struct list_node *add)
{
	if (!head || !add)
		return -1;

	add->next = head->next;
	head->next = add;

	return 0;
}

//TODO: Note caller has to deallocate struct list_node *rm after calling this function

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
	if (!data || !head)
		return false;

	for (head; head != NULL; head = head->next)
		if (is_equal(head->data, data))
			return true;

	return false;
}

#if 0
int main(int argc, char *argv[])
{
	struct list_node *head = NULL;

	struct list_node *tmp = NULL;
	struct channel c1, c2, c3;

	strncpy(c1.name, "LinuxFTW!", sizeof("LINUXFTW!"));
	strncpy(c2.name, "WindowsFTL!", sizeof("WindowsFTL"));
	strncpy(c3.name, "GraduationFTW!", sizeof("GraduationFTW!"));

	head = calloc(1, sizeof(struct list_node));
	head->next = NULL;
	head->data = (void *)&c1;

	tmp = calloc(1, sizeof(struct list_node));
	tmp->next = head->next;
	head->next = tmp;
	tmp->data = (void *)&c2;

	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		printf("tmp->data = %s\n", ((struct channel *)(tmp->data))->name);
	}


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

