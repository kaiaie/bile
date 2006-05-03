/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Pair.h,v 1.4 2006/05/03 15:23:01 ken Exp $
 * Pair - A very simple name/value structure, used to implement Dict
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
