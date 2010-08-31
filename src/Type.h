/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Type.h,v 1.4 2010/08/31 15:11:58 ken Exp $
*/
/** \file Type.h
*** \brief Functions for testing and converting values stored in BILE variables
***
*** BILE stores all its values in strings (\sa Vars)
**/
#ifndef TYPE_H
#define TYPE_H
#include "bool.h"

bool   Type_isNumeric(char *val);
bool   Type_isDouble(char *val);
bool   Type_isLong(char *val);
bool   Type_toBool(char *val);
long   Type_toLong(char *val);
double Type_toDouble(char *val);

#endif /* TYPE_H */
