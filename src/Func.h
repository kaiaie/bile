/* :tabSize=4:indentSize=4:folding=indent: 
 * $Id: Func.h,v 1.14 2009/10/25 13:59:08 ken Exp $
*/
/**
 * \file Func.h
 * \brief intrinsic functions for expression parser.
 * All functions take a pointer to the expression's variable list and a 
 * variable-length list of arguments.  All functions should return a 
 * heap-allocated string.  Functions should not modify any arguments passed to 
 * them.  It is the responsibility of the caller to free() the string the 
 * function returns.
 */
#ifndef FUNC_H
#define FUNC_H
#include "Dict.h"
#include "List.h"
#include "Vars.h"
/* Sample BILE functions */
Dict *getFunctionList(void);

char *Func_length(Vars *v, List *args);
char *Func_substr(Vars *v, List *args);
char *Func_now(Vars *v, List *args);
char *Func_strftime(Vars *v, List *args);
char *Func_file(Vars *v, List *args);
char *Func_fileExists(Vars *v, List *args);
char *Func_tag(Vars *v, List *args);
char *Func_ent(Vars *v, List *args);
char *Func_exec(Vars *v, List *args);
char *Func_defined(Vars *v, List *args);
char *Func_basename(Vars *v, List *args);
char *Func_dirname(Vars *v, List *args);
char *Func_relativePath(Vars *v, List *args);
char *Func_indexFirst(Vars *v, List *args);
char *Func_indexPrev(Vars *v, List *args);
char *Func_indexNext(Vars *v, List *args);
char *Func_indexLast(Vars *v, List *args);
char *Func_decode(Vars *v, List *args);
char *Func_ucase(Vars *v, List *args);
char *Func_lcase(Vars *v, List *args);
char *Func_iif(Vars *v, List *args);
char *Func_indexof(Vars *v, List *args);


#endif /* FUNC_H */
