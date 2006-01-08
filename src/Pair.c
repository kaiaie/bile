/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Pair.c,v 1.2 2006/01/08 18:02:54 ken Exp $
 */
#include <stdlib.h>
#include "astring.h"
#include "Logging.h"
#include "memutils.h"
#include "Pair.h"

Pair *new_Pair(char *key, void *value){
	Pair *result = NULL;
	
	result = (Pair *)mu_malloc(sizeof(Pair));
	result->key   = astrcpy(key);
	result->value = value;
	return result;
}


void delete_Pair(Pair *p, bool freeData){
	if(p != NULL){
		if(p->key != NULL) free(p->key);
		if(freeData && p->value != NULL) free(p->value);
		free(p);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
}

