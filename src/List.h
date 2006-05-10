/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: List.h,v 1.3 2006/05/10 15:01:18 ken Exp $
 */
#ifndef _LIST_H
#define _LIST_H
#include <stddef.h>
#include "bool.h"

typedef struct list_node{
	void             *data;
	struct list_node *prev;
	struct list_node *next;
} ListNode;

typedef struct list_type{
	size_t   length;
	ListNode *first;
	ListNode *last;
	ListNode *curr;
} List;


typedef enum {LIST_FIRST, LIST_NEXT, LIST_PREV, LIST_LAST} ListDirection;


List *new_List();
void delete_List(List *l, bool freeData);

size_t List_length(List *l);
void  *List_get(List *l, long index);
bool   List_append(List *l, void *data);
bool   List_insert(List *l, long index, void *data);
bool   List_remove(List *l, long index, bool freeData);
bool   List_drop(List *l, bool freeData);
void  *List_current(List *l);
void  *List_previous(List *l);
void  *List_next(List *l);
size_t List_currentIndex(List *l);
bool   List_move(List *l, ListDirection d);
bool   List_moveFirst(List *l);
bool   List_moveLast(List *l);
bool   List_movePrevious(List *l);
bool   List_moveNext(List *l);
void **List_toArray(List *l, bool terminated);
bool   List_atStart(List *l);
bool   List_atEnd(List *l);

#endif /* _LIST_H */
