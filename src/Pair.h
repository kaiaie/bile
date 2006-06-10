/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Pair.h,v 1.5 2006/06/10 20:23:42 ken Exp $
 * Pair - A very simple name/value structure, used to implement Dict
 */
#ifndef PAIR_H
#define PAIR_H
#include "bool.h"

typedef struct _pair_type{
	char *key;
	void *value;
} Pair;

Pair *new_Pair(const char *key, void *value);
void delete_Pair(Pair *p, bool freeData);

#endif /* PAIR_H */
