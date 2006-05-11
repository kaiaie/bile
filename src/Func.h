/* :tabSize=4:indentSize=4:folding=indent: 
 * $Id: Func.h,v 1.10 2006/05/11 22:11:48 ken Exp $
 * Func - intrinsic functions for expression parser.
 * All functions take a pointer to the expression's variable list and two 
 * arguments a la the C main() function.  All functions 
 * should return a heap-allocated string.  Functions should not modify any 
 * strings passed to them.  It is the responsibility of the caller to free()
 * the string the function returns.
 */
#ifndef FUNC_H
#define FUNC_H
#include "Dict.h"
#include "Vars.h"
/* Sample BILE functions */
Dict *getFunctionList(void);

char *Func_length(Vars *v, int argc, char *argv[]);
char *Func_substr(Vars *v, int argc, char *argv[]);
char *Func_now(Vars *v, int argc, char *argv[]);
char *Func_strftime(Vars *v, int argc, char *argv[]);
char *Func_file(Vars *v, int argc, char *argv[]);
char *Func_fileExists(Vars *v, int argc, char *argv[]);
char *Func_tag(Vars *v, int argc, char *argv[]);
char *Func_ent(Vars *v, int argc, char *argv[]);
char *Func_exec(Vars *v, int argc, char *argv[]);
char *Func_defined(Vars *v, int argc, char *argv[]);
char *Func_basename(Vars *v, int argc, char *argv[]);
char *Func_dirname(Vars *v, int argc, char *argv[]);
char *Func_relativePath(Vars *v, int argc, char *argv[]);

#endif /* FUNC_H */
