/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Vars.h,v 1.8 2010/08/31 15:11:58 ken Exp $
*/
/**
*** \file Vars.h
*** \brief Storage and manipulation of BILE variables
*** 
*** BILE variables are stored in a linked list of Dicts, each Dict representing 
*** a "scope", with a pointer to the parent scope.  All variables are stored 
*** as strings, regardless of "type".
*** 
*** Attempting to change the value of a variable that exists in an outer 
*** scope will usually result in a copy of the variable being created in the 
*** local scope; this is to prevent side-effects as the order in which Bile 
*** processes its files is not defined.  The <tt>VAR_NOSHADOW</tt> flag prevents 
*** this from happening.
**/
#ifndef VARS_H
#define VARS_H
#include "Dict.h"

/** Properties of a variable */
typedef int VarFlags;

#define VAR_STD 0
/** Variable is a constant; warn if an attempt is made to redefine it */
#define VAR_CONST 1
/** Variable cannot be shadowed */
#define VAR_NOSHADOW 2

/** \brief The structure used to hold BILE variables */
typedef struct tag_vars{
	/** Pointer to enclosing scope */
	struct tag_vars *parent;
	/** The variables proper */
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
