/* :tabSize=4:indentSize=4:folding=indent: 
 * $Id: Func.h,v 1.2 2006/01/08 18:02:53 ken Exp $
 * Func - intrinsic functions for expression parser.
 * All functions take two arguments a la the C main() function.  All functions 
 * should return a heap-allocated string.  Functions should not modify any 
 * strings passed to them.  It is the responsibility of the caller to free()
 * the string the function returns.
 */
#ifndef _FUNC_H
#define _FUNC_H
/* Sample BILE functions */
char *Func_length(int argc, char *argv[]);
char *Func_substr(int argc, char *argv[]);

#endif /* _FUNC_H */
