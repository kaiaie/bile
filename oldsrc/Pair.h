#ifndef _PAIR_H
#define _PAIR_H
#include "bool.h"

typedef struct _pair{
   char *key;
   void *data;
} Pair;

Pair *new_Pair(char *key, void *data);
void delete_Pair(Pair *d, bool freeData);

#endif /* _DICT_H */
