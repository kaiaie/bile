#ifndef _VARS_H
#define _VARS_H
#include "bool.h"
#include "Dict.h"
typedef Dict Vars;

Vars *new_Vars(void);
void delete_Vars(Vars *v);

size_t Vars_length(Vars *v);
bool   Vars_exists(Vars *v, char *name);
char   *Vars_getString(Vars *v, char *name);
void   *Vars_getData(Vars *v, char *name);
void   Vars_putString(Vars *v, char *name, char *value);
void   Vars_putData(Vars *v, char *name, void *data);
void   Vars_dump(Vars *v);

#endif _VARS_H
