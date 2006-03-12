/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Vars.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 * vars - the data structure used to hold BILE variables.  Basically, it is a 
 * linked list of Dicts, each Dict representing a "scope" and a pointer to the 
 * parent scope. get() retrieves a value, creating a variable 
 * in the current scope if it doesn't exist; let() sets a 
 * value in the current scope; set() sets a value in the outermost "global" 
 * scope.
 */
#ifndef _VARS_H
#define _VARS_H
#include "Dict.h"

typedef struct _vars{
	struct _vars *parent;
	Dict *vars;
} Vars;

Vars *new_Vars(Vars *parent);
void delete_Vars(Vars *v);

char *Vars_get(Vars *v, char *name);
bool Vars_let(Vars *v, char *name, char *value);
bool Vars_set(Vars *v, char *name, char *value);
bool Vars_defined(Vars *v, char *name);

#endif /* VARS_H */
