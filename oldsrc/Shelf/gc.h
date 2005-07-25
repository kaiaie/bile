#ifndef _GC_H
#define _GC_H
#include <stddef.h>
#include "bool.h"

bool gcRegister(void (*compactor)(int level), void(*destructor)(void));
void gcCompact(int level);

void *gcMalloc(size_t size);
void *gcRealloc(void *block, size_t size);
void gcFree(void *block);

#endif /* _GC_H */
