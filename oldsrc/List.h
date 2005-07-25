#ifndef _LIST_H
#define _LIST_H
#include "bool.h"

typedef struct _list{
   size_t length;
   struct _list_node *first;
} List;

typedef struct _list_node{
   void *data;
   struct _list_node *next;
} ListNode;

List   *new_List(void);
void   delete_List(List *l, bool freeData);

size_t List_length(List *l);
void   List_insert(List *l, int index, void *data);
void   List_append(List *l, void *data);
void   List_remove(List *l, int index, bool freeData);

#endif
