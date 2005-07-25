/* :tabSize=4:indentSize=4:folding=indent: */
#ifndef _PAIR_H
#define _PAIR_H
#include "bool.h"

typedef struct _pair_type{
	char *key;
	void *value;
} Pair;

Pair *new_Pair(char *key, void *value);
void delete_Pair(Pair *p, bool freeData);

#endif /* _PAIR_H */
