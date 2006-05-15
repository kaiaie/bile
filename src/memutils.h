/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: memutils.h,v 1.3 2006/05/15 09:35:26 ken Exp $
 * memutils - Convenience functions for allocating and releasing memory.
 */
#ifndef MEMUTILS_H
#define MEMUTILS_H
#include <stddef.h>

void *mu__malloc(char *fileName, int lineNo, size_t size);
void *mu__realloc(char *fileName, int lineNo, void *ptr, size_t size);
void mu__free(char *fileName, int lineNo, void *ptr);

/* Use these macros rather than calling the allocation functions directly so 
 * that you will get an accurate location if an allocation fails.
 */
#define mu_malloc(size) mu__malloc(__FILE__, __LINE__, size)
#define mu_realloc(ptr, size) mu__realloc(__FILE__, __LINE__, ptr, size)
#define mu_free(ptr) mu__free(__FILE__, __LINE__, ptr)

#endif /* MEMUTILS_H */
