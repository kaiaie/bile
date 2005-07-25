#ifndef _LIST_H
#define _LIST_H
#include <stddef.h>
#include "bool.h"

#define INVALID_LIST 0
typedef int List;

List new_List();
void delete_List(List l);

bool List_appendItem(List l, int v);
bool List_deleteItem(List l, int pos);
bool List_insertItem(List l, int pos, int v);
bool List_setItem(List l, int pos, int v);

bool List_getItem(List l, int pos, int *v);
bool List_getLength(List l, size_t *lgt);

#endif /* _LIST_H */
