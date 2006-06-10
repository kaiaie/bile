/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Vars.h,v 1.5 2006/06/10 20:23:42 ken Exp $
 * vars - the data structure used to hold BILE variables.  Basically, it is a 
 * linked list of Dicts, each Dict representing a "scope" and a pointer to the 
 * parent scope. get() retrieves a value, creating a variable 
 * in the current scope if it doesn't exist; let() sets a 
 * value in the current scope; set() sets a value in the outermost "global" 
 * scope.
 */
#ifndef VARS_H
#define VARS_H
#include "Dict.h"

typedef int VarFlags;

#define VAR_STD 0
#define VAR_CONST 1
#define VAR_NOSHADOW 2

typedef struct _vars{
	struct _vars *parent;
	Dict *vars;
} Vars;

Vars *new_Vars(Vars *parent);
void delete_Vars(Vars *v);

char     *Vars_get(Vars *v, const char *name);
VarFlags Vars_getFlags(Vars *v, const char *name);
bool     Vars_let(Vars *v, const char *name, const char *value, VarFlags flags);
bool     Vars_set(Vars *v, const char *name, const char *value, VarFlags flags);
bool     Vars_defined(Vars *v, const char *name);

void Vars_dump(Vars *v);

#endif /* VARS_H */
