#include "list.h"
#include <stdio.h>
#include <stdlib.h>

static int add_list_node(struct list_node *head, struct list_node *new)
{
	if (!head || !new)
		return -1;

	new->next = head->next;
	head->next = new;

	return 0;
}

//TODO: Note caller has to deallocate struct list_node *rm after calling this function
static int rm_list_node(struct list_node *prev, struct list_node *rm)
{
	if (!prev || !rm)
		return -1;

	prev->next = rm->next;

	return 0;
}

int main(int argc, char *argv[])
{
	struct list_node *head = NULL;
	struct list_node *tmp = NULL;

	head = (struct list_node *)calloc(1, sizeof(struct list_node));
	head->next = NULL;
	head->data = 1;


	tmp = (struct list_node *)calloc(1, sizeof(struct list_node));
	tmp->next = head->next;
	head->next = tmp;
	tmp->data = 2;

	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		printf("tmp->data = %d\n", tmp->data);
	}

	tmp = head->next;
	rm_list_node(head, tmp);
	free(tmp);
	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		printf("tmp->data = %d\n", tmp->data);
	}

	free(head);
	return 0;
}


