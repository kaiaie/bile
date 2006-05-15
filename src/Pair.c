/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Pair.c,v 1.4 2006/05/15 09:35:26 ken Exp $
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
		mu_free(p->key);
		if(freeData) mu_free(p->value);
		mu_free(p);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
}

