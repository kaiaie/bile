/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Index.h,v 1.2 2010/08/31 15:11:57 ken Exp $
*/
#ifndef INDEX_H
#define INDEX_H
#include "BileObj.h"

Index   *new_Index(Section *parent, const char *name);
bool    Index_add(Index *idx, Story *st);
void Index_dump(Index *idx);

#endif /* INDEX_H */


