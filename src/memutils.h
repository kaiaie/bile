/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: memutils.h,v 1.1 2005/07/25 22:41:48 ken Exp $
 * memutils - Convenience functions for allocating and releasing memory.
 */
#ifndef _MEMUTILS_H
#define _MEMUTILS_H
#include <stddef.h>

void *mu__malloc(char *fileName, int lineNo, size_t size);
void *mu__realloc(char *fileName, int lineNo, void *ptr, size_t size);
void mu_free(void *ptr);

#define mu_malloc(size) mu__malloc(__FILE__, __LINE__, size)
#define mu_realloc(ptr, size) mu__realloc(__FILE__, __LINE__, ptr, size)

#endif /* _MEMUTILS_H */
