/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Vars.c,v 1.5 2006/05/03 15:21:09 ken Exp $
 */
#include <stdlib.h>
#include "astring.h"
#include "bool.h"
#include "Vars.h"
#include "Dict.h"
#include "Logging.h"
#include "memutils.h"

typedef enum {SCOPE_LOCAL, SCOPE_GLOBAL} Scope;

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
bool setVar(Vars *v, char *name, char *value, Scope scope){
	bool result = false;
	Vars *p     = NULL;
	
	if(v != NULL){
		p = v;
		if(scope == SCOPE_GLOBAL)
			while(p->parent != NULL) p = p->parent;
		if(Dict_exists(p->vars, name)) Dict_remove(p->vars, name, true);
		result = Dict_put(p->vars, name, value);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return result;
}


char *Vars_get(Vars *v, char *name){
	Vars *p = NULL;
	bool found = false;
	char *result = NULL;
	
	if(v != NULL){
		p = v;
		while(p != NULL){
			if(Dict_exists(p->vars, name)){
				result = Dict_get(p->vars, name);
				found = true;
				break;
			}
			p = p->parent;
		}
		if(!found){
			/* Is there an environment variable of this name? */
			if(getenv(name) != NULL){
				result = astrcpy(getenv(name));
			}
			else{
				/* Create an empty variable */
				result = astrcpy("");
			}
			/* Store in the current scope */
			Dict_put(v->vars, name, result);
		}
	}
	else
		Logging_warnNullArg(__FUNCTION__);
	return result;
}


bool Vars_let(Vars *v, char *name, char *value){
	return setVar(v, name, value, SCOPE_LOCAL);
}


bool Vars_set(Vars *v, char *name, char *value){
	return setVar(v, name, value, SCOPE_GLOBAL);
}


bool Vars_defined(Vars *v, char *name){
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

