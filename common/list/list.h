#ifndef _LIST_H
#define _LIST_H

struct list_node {
	struct list_node *next;
	int data;
};

int add_list_node(struct list_node *head, struct list_node *add);
int rm_list_node(struct list_node *prev, struct list_node *rm);

#endif /* _LIST_H */
