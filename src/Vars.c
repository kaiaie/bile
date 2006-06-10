/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Vars.c,v 1.6 2006/06/10 20:23:42 ken Exp $
 */
#include <stdlib.h>
#include "astring.h"
#include "bool.h"
#include "Vars.h"
#include "Dict.h"
#include "Logging.h"
#include "memutils.h"

typedef enum {SCOPE_LOCAL, SCOPE_GLOBAL} Scope;

typedef struct _var_rec{
	VarFlags flags;
	char     *value;
} VarRec;


VarRec *new_VarRec(VarFlags flags, const char *value){
	VarRec *result = mu_malloc(sizeof(VarRec));
	result->flags = flags;
	result->value = astrcpy(value);
	return result;
}


void delete_VarRec(VarRec *vr){
	if(vr != NULL){
		mu_free(vr->value);
		mu_free(vr);
	}
}


Vars *new_Vars(Vars *parent){
	Vars *v = NULL;
	v = mu_malloc(sizeof(Vars));
	v->parent = parent;
	v->vars = new_Dict();
	return v;
}


/* setVar - Set a variable in the local or global scope.
 * Note that this assumes that it can delete the variable if it already exists; 
 * for this reason, only heap-allocated copies of strings should be stored in 
 * variables.
 */
bool setVar(Vars *v, const char *name, const char *value, VarFlags flags, Scope scope){
	bool   result = false;
	Vars   *p     = NULL;
	VarRec *vr    = NULL;
	
	if(v != NULL){
		p = v;
		if(scope == SCOPE_GLOBAL)
			while(p->parent != NULL) p = p->parent;
		if(Dict_exists(p->vars, name)){
			delete_VarRec((VarRec *)Dict_get(p->vars, name));
			Dict_remove(p->vars, name, false);
		}
		vr = new_VarRec(flags, value);
		result = Dict_put(p->vars, name, vr);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return result;
}


char *Vars_get(Vars *v, const char *name){
	Vars   *p      = NULL;
	bool   found   = false;
	char   *result = NULL;
	VarRec *vr     = NULL;
	
	if(v != NULL){
		p = v;
		while(p != NULL){
			if(Dict_exists(p->vars, name)){
				result = ((VarRec *)Dict_get(p->vars, name))->value;
				found = true;
				break;
			}
			p = p->parent;
		}
		if(!found){
			/* Is there an environment variable of this name? */
			vr = new_VarRec(0, (getenv(name) != NULL) ? getenv(name) : "");
			result = vr->value;
			/* Store in the current scope */
			Dict_put(v->vars, name, vr);
		}
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return result;
} /* Vars_get */


VarFlags Vars_getFlags(Vars *v, const char *name){
	VarFlags result = VAR_STD;
	Vars     *p     = v;
	
	if(v != NULL){
		while(p != NULL){
			if(Dict_exists(p->vars, name)){
				result = ((VarRec *)Dict_get(p->vars, name))->flags;
				break;
			}
			p = p->parent;
		}
	}
	return result;
} /* Vars_getFlags */


bool Vars_let(Vars *v, const char *name, const char *value, VarFlags flags){
	return setVar(v, name, value, flags, SCOPE_LOCAL);
}


bool Vars_set(Vars *v, const char *name, const char *value, VarFlags flags){
	return setVar(v, name, value, flags, SCOPE_GLOBAL);
}


bool Vars_defined(Vars *v, const char *name){
	bool result = false;
	Vars *p = NULL;
	
	if(v != NULL){
		p = v;
		while(p != NULL){
			if(Dict_exists(p->vars, name)){
				result = true;
				break;
			}
			p = p->parent;
		}
		if(!result){
			/* Last resort: try environment variables */
			result = (getenv(name) != NULL);
		}
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return result;
}


void delete_Vars(Vars *v){
	if(v != NULL){
		delete_Dict(v->vars, true);
		mu_free(v);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
}


void Vars_dump(Vars *v){
	Dict_dump(v->vars, "\t");
}

