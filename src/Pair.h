/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Pair.h,v 1.2 2006/01/08 18:02:54 ken Exp $
 */
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
