#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

typedef struct node node;

node* insert(node *head, void* data);
node* delete(node *head, void* data);

#endif