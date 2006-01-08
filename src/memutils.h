/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: memutils.h,v 1.2 2006/01/08 18:02:54 ken Exp $
 * memutils - Convenience functions for allocating and releasing memory.
 */
#ifndef _MEMUTILS_H
#define _MEMUTILS_H
#include <stddef.h>

void *mu__malloc(char *fileName, int lineNo, size_t size);
void *mu__realloc(char *fileName, int lineNo, void *ptr, size_t size);
void mu_free(void *ptr);

/* Use these macros rather than calling the allocation functions directly so 
 * that you will get an accurate location if an allocation fails.
 */
#define mu_malloc(size) mu__malloc(__FILE__, __LINE__, size)
#define mu_realloc(ptr, size) mu__realloc(__FILE__, __LINE__, ptr, size)

#endif /* _MEMUTILS_H */
