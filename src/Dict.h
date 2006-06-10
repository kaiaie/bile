/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Dict.h,v 1.4 2006/06/10 20:23:42 ken Exp $
 */
#ifndef DICT_H
#define DICT_H
#include "bool.h"
#include "List.h"

typedef List Dict;

Dict *new_Dict();
void delete_Dict(Dict *d, bool freeData);

size_t Dict_length(Dict *d);
bool Dict_exists(Dict *d, const char *key);
bool Dict_put(Dict *d, const char *key, void *value);
bool Dict_putSorted(Dict *d, const char *key, void *value);
void *Dict_get(Dict *d, const char *key);
bool Dict_remove(Dict *d, const char *key, bool freeData);

/* Debug functions */
void Dict_dump(Dict *d, char *prefix);

#endif /* DICT_H */
