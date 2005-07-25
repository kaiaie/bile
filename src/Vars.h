/* :tabSize=4:indentSize=4:folding=indent:
 * vars - the data structure used to hold BILE variables.  Basically, it is a 
 * List of Dicts, each Dict representing a "scope", the first element in the 
 * List being the current scope.  get() retrieves a value, creating a variable 
 * in the current scope if it doesn't exist; let() sets a 
 * value in the current scope; set() sets a value in the outermost "global" 
 * scope.
 */
#ifndef _VARS_H
#define _VARS_H
#include "List.h"

typedef List Vars;

char *Vars_get(Vars *v, char *name);
bool Vars_let(Vars *v, char *name, char *value);
bool Vars_set(Vars *v, char *name, char *value);
bool Vars_defined(Vars *v, char *name);

#endif _VARS_H
