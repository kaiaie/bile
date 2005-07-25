/* :tabSize=4:indentSize=4:folding=indent: */
#ifndef _DICT_H
#define _DICT_H
#include "bool.h"
#include "List.h"

typedef List Dict;

Dict *new_Dict();
void delete_Dict(Dict *d, bool freeData);

size_t Dict_length(Dict *d);
bool Dict_exists(Dict *d, char *key);
bool Dict_put(Dict *d, char *key, void *value);
void *Dict_get(Dict *d, char *key);
bool Dict_remove(Dict *d, char *key, bool freeData);

/* Debug functions */
void Dict_dump(Dict *d, char *prefix);

#endif /* _DICT_H */
