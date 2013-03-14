/* :tabSize=4:indentSize=4:folding=indent: 
** $Id: Func.h,v 1.16 2010/08/31 15:11:57 ken Exp $
*/
/**
*** \file Func.h
*** \brief intrinsic functions for expression parser
***
*** All functions take a pointer to the expression's variable list and a 
*** variable-length list of arguments.  All functions should return a 
*** heap-allocated string.  Functions should not modify any arguments passed to 
*** them.  It is the responsibility of the caller to free() the string the 
*** function returns.
**/
#ifndef FUNC_H
#define FUNC_H
#include "BileObj.h"

Dict *getFunctionList(void);

char *Func_length(BileObject *context, List *args);
char *Func_substr(BileObject *context, List *args);
char *Func_now(BileObject *context, List *args);
char *Func_strftime(BileObject *context, List *args);
char *Func_file(BileObject *context, List *args);
char *Func_fileExists(BileObject *context, List *args);
char *Func_tag(BileObject *context, List *args);
char *Func_ent(BileObject *context, List *args);
char *Func_exec(BileObject *context, List *args);
char *Func_defined(BileObject *context, List *args);
char *Func_basename(BileObject *context, List *args);
char *Func_dirname(BileObject *context, List *args);
char *Func_relativePath(BileObject *context, List *args);
char *Func_indexFirst(BileObject *context, List *args);
char *Func_indexPrev(BileObject *context, List *args);
char *Func_indexNext(BileObject *context, List *args);
char *Func_indexLast(BileObject *context, List *args);
char *Func_decode(BileObject *context, List *args);
char *Func_ucase(BileObject *context, List *args);
char *Func_lcase(BileObject *context, List *args);
char *Func_iif(BileObject *context, List *args);
char *Func_indexof(BileObject *context, List *args);


#endif /* FUNC_H */
