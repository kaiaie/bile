#ifndef _ATTRLIST_H
#define _ATTRLIST_H
#include <stddef.h>
#include "bool.h"
#include "str.h"

#define INVALID_ATTRLIST 0
typedef int AttrList;

AttrList new_List();
void delete_AttrList(AttrList l);

bool AttrList_deleteItem(AttrList l, String key);
bool AttrList_replaceItem(AttrList l, String key, int v);
bool AttrList_setItem(AttrList l, String key, int v);

bool List_getItem(AttrList l, String key, int *v);
bool List_getLength(AttrList l, size_t *lgt);

#endif /* _ATTRLIST_H */
