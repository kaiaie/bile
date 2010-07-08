/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Tags.h,v 1.1 2010/07/08 21:04:24 ken Exp $
 */
#ifndef TAGS_H
#define TAGS_H
#include "bool.h"
#include "BileObj.h"

Tags *new_Tags(Publication *parent, const char *name);
bool Tags_add(Tags *t, Story *st);

#endif /* TAGS_H */


