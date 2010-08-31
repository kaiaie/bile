/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Pair.h,v 1.8 2010/08/31 15:11:57 ken Exp $
*/
/**
*** \file Pair.h
*** \brief A very simple name/value structure, used to implement Dict
*** \sa Dict
***
**/
#ifndef PAIR_H
#define PAIR_H
#include "bool.h"

typedef struct tag_pair_type {
	char *key;
	void *value;
} Pair;

Pair *new_Pair(const char *key, void *value);
void delete_Pair(Pair *p, bool freeData);

#endif /* PAIR_H */
