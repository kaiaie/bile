/* :tabSize=4:indentSize=4:folding=indent: */
#include <stdlib.h>
#include "astring.h"
#include "bool.h"
#include "Vars.h"
#include "Dict.h"
#include "List.h"
#include "Logging.h"

bool setVar(Vars *v, char *name, char *value, int scope){
	bool result = false;
	Dict *d = NULL;
	
	if(v != NULL){
		d = (Dict *)List_get((List *)v, scope);
		result = Dict_put(d, name, value);
	}
	else{
		Logging_warnf("%s: NULL argument", __FUNCTION__);
	}
	return result;
}


char *Vars_get(Vars *v, char *name){
	List *l = NULL;
	Dict *d = NULL;
	size_t ii;
	bool found = false;
	char *result = NULL;
	
	if(v != NULL){
		l = (List *)v;
		for(ii = 0; ii < List_length(l); ++ii){
			d = (Dict *)List_get(l, ii);
			if(Dict_exists(d, name)){
				result = (char *)Dict_get(d, name);
				found = true;
				break;
			}
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
			d = (Dict *)List_get(l, 0);
			Dict_put(d, name, result);
		}
	}
	else{
		Logging_warnf("%s: NULL argument", __FUNCTION__);
	}
	return result;
}


bool Vars_let(Vars *v, char *name, char *value){
	return setVar(v, name, value, 0);
}


bool Vars_set(Vars *v, char *name, char *value){
	return setVar(v, name, value, -1);
}


bool Vars_defined(Vars *v, char *name){
	bool found = false;
	bool result = false;
	List *l = NULL;
	Dict *d = NULL;
	size_t ii;
	
	if(v != NULL){
		l = (List *)v;
		for(ii = 0; ii < List_length(l); ++ii){
			d = (Dict *)List_get(l, ii);
			if(Dict_exists(d, name){
				found = true;
				break;
			}
			if(!found){
				/* Try environment variables */
				found = (getenv(name) != NULL);
			}
			result = found;
		}
	}
	else
		Logging_warnf("%s: NULL argument", __FUNCTION__);
	return result;
}
