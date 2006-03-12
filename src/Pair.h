/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Pair.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 */
#ifndef PAIR_H
#define PAIR_H
#include "bool.h"

typedef struct _pair_type{
	char *key;
	void *value;
} Pair;

Pair *new_Pair(char *key, void *value);
void delete_Pair(Pair *p, bool freeData);

#endif /* PAIR_H */
