/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Vars.c,v 1.13 2010/08/31 15:11:58 ken Exp $
*/
#include <stdlib.h>
#include "astring.h"
#include "bool.h"
#include "Vars.h"
#include "Dict.h"
#include "Logging.h"
#include "Pair.h"
#include "memutils.h"
#include "stringext.h"

typedef enum {SCOPE_LOCAL, SCOPE_GLOBAL} Scope;

/** Structure to hold the variable's details */
typedef struct tag_var_rec {
	VarFlags flags;
	char     *value;
} VarRec;


VarRec *new_VarRec(VarFlags flags, const char *value) {
	VarRec *result = mu_malloc(sizeof(VarRec));
	result->flags = flags;
	result->value = astrcpy(value);
	return result;
}


void delete_VarRec(VarRec *vr) {
	if (vr != NULL) {
		mu_free(vr->value);
		mu_free(vr);
	}
}


/**
*** \brief Creates a new scope within the parent scope
**/
Vars *new_Vars(Vars *parent) {
	Vars *v = NULL;
	v = mu_malloc(sizeof(Vars));
	v->parent = parent;
	v->vars = new_Dict();
	return v;
}


/**
*** \brief Sets a variable in the local or global scope.
**/
bool setVar(Vars *v, const char *name, const char *value, VarFlags flags, Scope scope) {
	bool   result = false;
	Vars   *p     = NULL;
	VarRec *vr    = NULL;
	
	if (v != NULL){
		p = v;
		if (scope == SCOPE_GLOBAL) {
			while (p->parent != NULL) p = p->parent;
		}
		else {
			if (p->parent != NULL && 
				Vars_defined(p->parent, name) && 
				(Vars_getFlags(p->parent, name) & VAR_NOSHADOW)
			) {
				p = p->parent;
				while (true) {
					if (Dict_exists(p->vars, name)) break;
					p = p->parent;
				}
			}
		}
		if (Dict_exists(p->vars, name)){
			if (((VarRec *)Dict_get(p->vars, name))->flags & VAR_CONST) {
				return false;
			}
			else {
				delete_VarRec((VarRec *)Dict_get(p->vars, name));
				Dict_remove(p->vars, name, false);
			}
		}
		vr = new_VarRec(flags, value);
		result = Dict_put(p->vars, name, vr);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
	return result;
}


/**
*** \brief Get the value of a variable
***
*** If the variable does not exist in the local scope, the enclosing scopes are 
*** searched, followed by the system environment.
***
**/
char *Vars_get(Vars *v, const char *name) {
	Vars   *p      = NULL;
	bool   found   = false;
	char   *result = NULL;
	VarRec *vr     = NULL;
	char   *tmp    = NULL;
	
	if (v != NULL) {
		p = v;
		tmp = astrcpy(name);
		strxlower(tmp);
		while (p != NULL) {
			if (Dict_exists(p->vars, tmp)) {
				result = ((VarRec *)Dict_get(p->vars, tmp))->value;
				found = true;
				break;
			}
			p = p->parent;
		}
		if (!found) {
			/* Is there an environment variable of this name? */
			vr = new_VarRec(0, (getenv(name) != NULL) ? getenv(name) : "");
			result = vr->value;
			/* Store in the current scope */
			Dict_put(v->vars, name, vr);
		}
		mu_free(tmp);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
	return result;
} /* Vars_get */


VarFlags Vars_getFlags(Vars *v, const char *name){
	VarFlags result = VAR_STD;
	Vars     *p     = v;
	
	if (v != NULL) {
		while (p != NULL) {
			if (Dict_exists(p->vars, name))  {
				result = ((VarRec *)Dict_get(p->vars, name))->flags;
				break;
			}
			p = p->parent;
		}
	}
	return result;
} /* Vars_getFlags */


/**
*** \brief Sets the value of a variable
***
*** If the variable is defined outside the local scope, a modified copy is 
*** created in the local scope to prevent side-effects (unless the VAR_NOSHADOW 
*** flag has been set on the existing variable)
**/
bool Vars_let(Vars *v, const char *name, const char *value, VarFlags flags) {
	return setVar(v, name, value, flags, SCOPE_LOCAL);
}


/**
*** \brief Sets the value of a global variable
**/
bool Vars_set(Vars *v, const char *name, const char *value, VarFlags flags) {
	return setVar(v, name, value, flags, SCOPE_GLOBAL);
}


/**
*** \brief Determines if a variable has been defined in any scope
**/
bool Vars_defined(Vars *v, const char *name) {
	bool result = false;
	char *tmp = NULL;
	Vars *p = NULL;
	
	if (v != NULL) {
		tmp = astrcpy(name);
		strxlower(tmp);
		p = v;
		while (p != NULL) {
			if (Dict_exists(p->vars, tmp)) {
				result = true;
				break;
			}
			p = p->parent;
		}
		if (!result) {
			/* Last resort: try environment variables */
			result = (getenv(name) != NULL);
		}
		mu_free(tmp);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
	return result;
}


void delete_Vars(Vars *v) {
	if (v != NULL) {
		delete_Dict(v->vars, true);
		mu_free(v);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


void Vars_dump(Vars *v) {
	List *l = NULL;
	Dict *d = new_Dict();
	Pair *p = NULL;
	VarRec *r;
	int ii;
	
	l = (List *)v->vars;
	for (ii = 0; ii < List_length(l); ++ii) {
		p = (Pair *)List_get(l, ii);
		r = (VarRec *)p->value;
		Dict_put(d, p->key, r->value);
	}	
	Dict_dump(d, "\t");
	delete_Dict(d, false);
}

