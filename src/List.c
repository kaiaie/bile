/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: List.c,v 1.8 2006/12/13 22:57:57 ken Exp $
 */
#include <assert.h>
#include <stdlib.h>
#include "List.h"
#include "Logging.h"
#include "memutils.h"

/**
 * \file List.h
 * \note Indexing of List elements
 *
 * A List contains a number of items.  You can access an item in the list 
 * with List_get(): List_get(0) returns the first item, List_get(1) returns 
 * the second item, etc.  If you specify a negative argument, it counts from 
 * the end of the list, so List_get(-1) returns the last item, List_get(-2) 
 * returns the second-to-last item, etc.  The same numbering is used for 
 * List_remove()
 *
 * For List_insert(), the indexing is slightly different; in this case, the 
 * index number denotes the "space" between the items.  Or to put it another 
 * way, after the call to List_insert(), the inserted item will have the 
 * specified index.
 *
 * Example for a List with 4 items in it:
 *
 * \code
 * insert: 0  1  2  3  4
 *        -5 -4 -3 -2 -1
 *         +--+--+--+--+
 * get:    | 0| 1| 2| 3|
 *         |-4|-3|-2|-1|
 *         +--+--+--+--+
 * \endcode
 */

List *newList(){
	List *l = NULL;
	
	l = (List *)mu_malloc(sizeof(List));
	l->length = 0;
	l->first  = NULL;
	l->last   = NULL;
	l->curr   = NULL;
	return l;
}


ListNode *newNode(void *data){
	ListNode *n = NULL;
	
	n = (ListNode *)mu_malloc(sizeof(ListNode));
	n->data = data;
	n->prev = NULL;
	n->next = NULL;
	return n;
}


size_t getInsertIndex(List *l, long idx){
	long tmp = 0;
	
	tmp = (idx >= 0 ? idx : idx + l->length + 1);
	if(tmp >= 0 && tmp <= l->length)
		return tmp;
	else{
		Logging_warnf("%s(): Index out of range", __FUNCTION__);
		return 0;
	}
}


size_t getIndex(List *l, long idx){
	long tmp = 0;
	
	tmp = (idx >= 0 ? idx : idx + l->length);
	if(tmp >= 0 && tmp < l->length)
		return tmp;
	else{
		Logging_warnf("%s(): Index out of range", __FUNCTION__);
		return 0;
	}
}


bool findData(List *l, void *data, size_t *idx){
	ListNode *p = l->first;
	size_t   nodeCount = 0;
	
	while(p != NULL){
		if(p->data == data){
			if(idx != NULL) *idx = nodeCount;
			return true;
		}
		nodeCount++;
		p = p->next;
	}
	return false;
}


/** 
 * \brief Creates a new empty List
 */
List *new_List(){
	return newList();
}


/**
 * \brief Destroys a List
 * \param l The List to destroy
 * \param freeData if true, free the elements in the list also
 */
void delete_List(List *l, bool freeData){
	if(l != NULL){
		while(l->length > 0) List_remove(l, 0L, freeData);
		mu_free(l);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
}


/**
 * \brief Returns the number of elements in the list
 * \param l The List
 */
size_t List_length(List *l){
	return l->length;
}


/**
 * \brief Returns an element of the list
 * \param l The List
 */
void *List_get(List *l, long index){
/* Gets the nth item in the list. */
	void *retVal = NULL;
	size_t idx = getIndex(l, index);
	ListNode *itr = NULL;
	
	if(idx == 0)
		retVal = l->first->data;
	else if(idx == l->length - 1)
		retVal = l->last->data;
	else{
		itr = l->first;
		while(idx > 0){
			itr = itr->next;
			idx--;
		}
		retVal = itr->data;
	}
	return retVal;
}


/**
 * \brief Inserts an item at the end of the list
 */
bool List_append(List *l, void *data){
	return List_insert(l, -1L, data);
}


/**
 * \brief Inserts an item at the specified position in the list
 */
bool List_insert(List *l, long index, void *data){
/* Inserts an item into the list. */
	bool retVal = false;
	size_t idx  = getInsertIndex(l, index);
	ListNode *t = NULL;
	ListNode *n = newNode(data);

	if(l->length == 0){
		/* First node in list */
		l->first = n;
		l->last  = n;
	}
	else{
		if(idx == 0){
			/* Insert before first node */
			t = l->first;
			l->first = n;
			t->prev = n;
			n->next = t;
		}
		else if(idx == l->length){
			/* Insert after last node */
			t = l->last;
			l->last = n;
			t->next = n;
			n->prev = t;
		}
		else{
			t = l->first;
			while(idx > 0){
				t = t->next;
				idx--;
			}
			n->prev = t->prev;
			n->prev->next = n;
			n->next = t;
			n->next->prev = n;
		}
	}
	l->curr = n;
	l->length++;
	retVal = true;
	return retVal;
}


/**
 * \brief Removes the item at the specified position in the list, optionally 
 * freeing it
 */
bool List_remove(List *l, long index, bool freeData){
/* Removes an item from the list, optionally freeing its data. */
	bool retVal = false;
	size_t idx  = getIndex(l, index);
	ListNode *t = NULL;
	
	if(idx == 0){
		/* Delete first node */
		t = l->first;
		l->first = l->first->next;
		if(l->first != NULL) l->first->prev = NULL;
		if(l->curr == t) l->curr = l->first;
	}
	else if(idx == l->length - 1){
		/* Delete last node */
		t = l->last;
		l->last = l->last->prev;
		l->last->next = NULL;
		if(l->curr == t) l->curr = l->last;
	}
	else{
		t = l->first;
		while(idx > 0){
			t = t->next;
			idx--;
		}
		t->prev->next = t->next;
		t->next->prev = t->prev;
		if(l->curr == t){
			if(t->next != NULL)
				l->curr = t->next;
			else if(t->prev != NULL)
				l->curr = t->prev;
			else
				l->curr = NULL;
		}
	}
	if(freeData) mu_free(t->data);
	mu_free(t);
	l->length--;
	retVal = true;
	return retVal;
}


/**
 * \brief Removes the last element in the list
 */
bool List_drop(List *l, bool freeData){
	return List_remove(l, -1, freeData);
}


/**
 * \brief Returns the element of the list currently pointed to by its internal 
 * pointer
 */
void *List_current(List *l){
	void *retVal = NULL;
	if(l != NULL)
		retVal = l->curr->data;
	else
		Logging_warnf("%s(): NULL argument", __FUNCTION__);
	return retVal;
}


/**
 * \brief Returns the element of the list before the one pointed to by its 
 * internal pointer.
 *
 * \note Calling this function does not move the internal pointer.
 */
void  *List_getPrevious(List *l){
	void *retVal = NULL;
	
	if(l != NULL){
		if(l->curr != l->first)
			retVal = l->curr->prev->data;
		else
			Logging_warnf("%s(): At start of list", __FUNCTION__);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;
}


/**
 * \brief Returns the element of the list after the one pointed to by its 
 * internal pointer.
 *
 * \note Calling this function does not move the internal pointer.
 */
void  *List_getNext(List *l){
	void *retVal = NULL;
	
	if(l != NULL){
		if(l->curr != l->last)
			retVal = l->curr->next->data;
		else
			Logging_warnf("%s(): At end of list", __FUNCTION__);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;
}



/**
 * \brief Returns the index of the element of the list pointed to by its 
 * internal pointer.
 */
size_t List_currentIndex(List *l){
	size_t   retVal = 0;
	ListNode *p     = NULL;
	
	if(l != NULL){
		if(l->first != NULL){
			p = l->first;
			while(p != NULL){
				if(p == l->curr) break;
				p = p->next;
				retVal++;
			}
		}
		else
			Logging_warnf("%s(): Empty list", __FUNCTION__);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;	
}


/**
 * \brief Moves the list's internal pointer
 */
bool List_move(List *l, ListDirection d){
	bool retVal = false;
	ListNode *n = NULL;
	
	if(l != NULL){
		switch(d){
			case(LIST_FIRST):
				n = l->first;
				break;
			case(LIST_LAST):
				n = l->last;
				break;
			case(LIST_NEXT):
				n = l->curr->next;
				break;
			case(LIST_PREV):
				n = l->curr->prev;
				break;
		}
		if(n != NULL){
			l->curr = n;
			retVal = true;
		}
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;
}


/**
 * \brief Moves the internal pointer to the first item in the list
 */
bool List_moveFirst(List *l){
	return List_move(l, LIST_FIRST);
}


/**
 * \brief Moves the internal pointer to the last item in the list
 */
bool List_moveLast(List *l){
	return List_move(l, LIST_LAST);
}


/**
 * \brief Moves the internal pointer to the previous item in the list
 */
bool List_movePrevious(List *l){
	return List_move(l, LIST_PREV);
}


/**
 * \brief Moves the internal pointer to the next item in the list
 */
bool List_moveNext(List *l){
	return List_move(l, LIST_NEXT);
}


/**
 * \brief Returns an array of pointers to each element in the list, optionally 
 * terminated with NULL.
 *
 * \note It is the responsibility of the caller to free() the array once the 
 * caller is finished with it.
 */
void **List_toArray(List *l, bool terminated){
	void **retVal = NULL;
	size_t arraySize;
	size_t ii;
	
	if(l != NULL){
		/* If the list is empty and an unterminated array is requested, the 
		 * function returns NULL.  If a terminated array is requested, the 
		 * function returns a single-element containing a NULL pointer.
		 */
		if(!terminated && List_length(l) == 0) return retVal;
		arraySize = List_length(l);
		if(terminated) arraySize++;
		retVal = (void **)mu_malloc(arraySize * sizeof(void *));
		for(ii = 0; ii < List_length(l); ++ii)
			retVal[ii] = List_get(l, ii);
		if(terminated) retVal[ii] = NULL;
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;
}


/**
 * \brief Returns true if the internal pointer is at the start of the list
 */
bool List_atStart(List *l){
	bool retVal = false;
	
	if(l != NULL)
		retVal = (l->curr == l->first);
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;
}


/**
 * \brief Returns true if the internal pointer is at the end of the list
 */
bool List_atEnd(List *l){
	bool retVal = false;
	
	if(l != NULL)
		retVal = (l->curr == l->last);
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;
}


/**
 * \brief Returns True if the list contains the specified data, False otherwise
 */
bool List_contains(List *l, void *data){
	if(l != NULL)
		return findData(l, data, (size_t *)NULL);
	else
		Logging_warnNullArg(__FUNCTION__);
	return false;
}


/**
 * \brief Returns the index of the specified data.
 *
 * \note List_contains should always be called before calling this function or a 
 * fatal error will be occur if the data is not in the list.
 */
size_t List_indexOf(List *l, void *data){
	size_t retVal = 0;
	if(l != NULL){
		if(!findData(l, data, &retVal))
			Logging_fatal("Data not in list!");
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return retVal;
}

