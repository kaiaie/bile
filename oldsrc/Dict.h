#ifndef _DICT_H
#define _DICT_H
#include "bool.h"
#include "List.h"
typedef List Dict;

Dict *new_Dict(void);
void delete_Dict(Dict *d, bool freeData);

void   *Dict_get(Dict *d, char *key);
bool   Dict_exists(Dict *d, char *key);
size_t Dict_length(Dict *d);
void   Dict_put(Dict *d, char *key, void *data);
void Dict_remove(Dict *d, char *key, bool freeData);

#endif /* _DICT_H */
