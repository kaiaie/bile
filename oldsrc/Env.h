#ifndef _ENV_H
#define _ENV_H
#include "Tree.h"

typedef Tree Env;

Env  *new_Env(Env *parent);
void *Env_get(Env *e, char *key);
char *Env_getString(Env *e, char *key);
void Env_set(Env *e, char *key, void *data);
void Env_setString(Env *e, char *key, char *data);

#endif /* _ENV_H */
