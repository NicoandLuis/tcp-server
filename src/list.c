#include <stdio.h>
#include <stdlib.h>
#include "../lib/list.h"

typedef struct node
{
    struct node *next;
    void* value;
} node;

//typedef struct list {
//    node *head;
//} list;

//void print_list(node *head)
//{
//    node *cursor = head;
//    while(cursor != NULL) {
//        printf("%d\n", cursor->value);
//        cursor = cursor->next;
//    }
//}


// inserts the node at the end of the list and returns that node
node* insert(node *head, void* data) {

    // if list is empty, node becomes the head
    if (head == NULL) {
        node *cursor;
        cursor = malloc(sizeof(node));
        cursor->next = NULL;
        cursor->value = data;
        head = cursor;
        
        return cursor;
    }

    // iterate to the last node and append the new one
    else {
        while(head->next != NULL) {
            head = head->next;
        }
        node *temp;
        temp = malloc(sizeof(node));
        temp->next = NULL;
        temp->value = data;

        return temp;
    }
}


// deletes the given node and returns the head
node* delete(node *head, void* data) {
    node* cursor = head;

    if(head->value == data)
    {
        cursor = head->next;
        free(head);

        return cursor;
    }
    // iterate through list until we find node to delete
    else {

        int i = 0;
        while (cursor->next != NULL) {
            cursor = cursor->next;
            i++;
        }

        // iterate through list again to find the node before the one to delete
        node* prev = head;

        for(int j = 0; j < i; j++) {
            prev = prev->next;
        }

        // change the pointer of prev to the next node of our node to delete
        prev->next = cursor->next;
        // free the node to delete
        free(cursor);

        return head;
    }
}


node* get(node* head, void* value) {
    // if head is what we search, return head
    if(head->value == value) return head;

    while(head->next != NULL) {
        if(head->next->value == value) return head->next;
    }

    return NULL;
}


//void printLast(node *tail) {
//    printf("%d", list->tail;
//}
