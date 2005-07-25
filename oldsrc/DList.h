#ifndef _DLIST_H
#define _DLIST_H
#include "bool.h"

typedef struct _dlist{
   void *data;
   struct _dlist *prev;
   struct _dlist *next;
} DList;

DList  *new_DList(void);
void   delete_DList(DList *d, bool freeData);
size_t DList_length(DList *dl);

#endif /* _DLIST_H */
