#ifndef INDEX_H
#define INDEX_H
#include "BileObj.h"

Index   *new_Index(Section *parent, const char *name);
bool    Index_add(Index *idx, Story *st);
void Index_dump(Index *idx);

#endif /* INDEX_H */


