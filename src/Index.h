/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Index.h,v 1.1 2010/07/08 21:04:24 ken Exp $
 */
#ifndef INDEX_H
#define INDEX_H
#include "BileObj.h"

Index   *new_Index(Section *parent, const char *name);
bool    Index_add(Index *idx, Story *st);
void Index_dump(Index *idx);

#endif /* INDEX_H */


