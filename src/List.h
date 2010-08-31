/* :tabSize=4:indentSize=4:folding=indent:
** $Id: List.h,v 1.8 2010/08/31 15:11:57 ken Exp $
*/
/**
*** \file List.h
*** \brief Implements a doubly-linked list
***
*** This is a standard doubly-linked list implementation.  It can be accessed 
*** like an array but also has an internal pointer (similar to a PHP array) so 
*** that it can be iterated.
**/
#ifndef LIST_H
#define LIST_H
#include <stddef.h>
#include "bool.h"

/** Represents each node in the list */
typedef struct list_node{
	void             *data;
	struct list_node *prev;
	struct list_node *next;
} ListNode;

/** \brief Represents the list itself 
***
*** Having a separate structure instead of returning a pointer to the first 
*** node in the list has two advantages: it allows for the first node to be 
*** deleted without problems, and gives us a place to store a length field.
**/
typedef struct list_type{
	size_t   length;
	ListNode *first;
	ListNode *last;
	ListNode *curr;
} List;

/** Constants used to indicate how List_move should move the internal pointer */
typedef enum {
	/** Moves pointer to the start of the list */
	LIST_FIRST, 
	/** Move pointer to the next element of the list */
	LIST_NEXT, 
	/** Moves pointer to the previous element of the list */
	LIST_PREV, 
	/** Moves pointer to the last element of the list */
	LIST_LAST
} ListDirection;


List *new_List();
void delete_List(List *l, bool freeData);

size_t List_length(List *l);
void  *List_get(List *l, long index);
bool   List_append(List *l, void *data);
bool   List_insert(List *l, long index, void *data);
bool   List_remove(List *l, long index, bool freeData);
bool   List_drop(List *l, bool freeData);
void  *List_current(List *l);
void  *List_getPrevious(List *l);
void  *List_getNext(List *l);
size_t List_currentIndex(List *l);
bool   List_move(List *l, ListDirection d);
bool   List_moveFirst(List *l);
bool   List_moveLast(List *l);
bool   List_movePrevious(List *l);
bool   List_moveNext(List *l);
void **List_toArray(List *l, bool terminated);
bool   List_atStart(List *l);
bool   List_atEnd(List *l);
bool   List_contains(List *l, void *data);
size_t List_indexOf(List *l, void *data);

#define List_getString(a, b) (char *)List_get(a, b)
#define List_currentString(a) (char *)List_current(a)

#endif /* LIST_H */
