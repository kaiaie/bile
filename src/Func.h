/* :tabSize=4:indentSize=4:folding=indent: 
 * $Id: Func.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 * Func - intrinsic functions for expression parser.
 * All functions take two arguments a la the C main() function.  All functions 
 * should return a heap-allocated string.  Functions should not modify any 
 * strings passed to them.  It is the responsibility of the caller to free()
 * the string the function returns.
 */
#ifndef FUNC_H
#define FUNC_H
/* Sample BILE functions */
char *Func_length(int argc, char *argv[]);
char *Func_substr(int argc, char *argv[]);

#endif /* FUNC_H */
