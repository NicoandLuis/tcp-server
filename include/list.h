#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include <stdio.h>

typedef struct node node;

node* insert(node *head, void* data);
node* delete(node *head, void* data);

#endif