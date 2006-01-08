/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Type.h,v 1.2 2006/01/08 18:02:54 ken Exp $
 */
#ifndef _TYPE_H
#define _TYPE_H
#include "bool.h"

bool   Type_isNumeric(char *val);
bool   Type_isDouble(char *val);
bool   Type_isLong(char *val);
bool   Type_toBool(char *val);
long   Type_toLong(char *val);
double Type_toDouble(char *val);

#endif /* _TYPE_H */
