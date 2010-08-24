/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Pair.c,v 1.6 2010/08/24 22:10:37 ken Exp $
 */
#include <stdlib.h>
#include "astring.h"
#include "Logging.h"
#include "memutils.h"
#include "Pair.h"

/** Creates a new Pair with the supplied key and value */
Pair *new_Pair(const char *key, void *value){
	Pair *result = NULL;
	
	result = (Pair *)mu_malloc(sizeof(Pair));
	result->key   = astrcpy(key);
	result->value = value;
	return result;
}

/** Destroys a Pair and frees the associated storage */
void delete_Pair(Pair *p, bool freeData){
	if (p != NULL) {
		mu_free(p->key);
		if (freeData) mu_free(p->value);
		mu_free(p);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}

