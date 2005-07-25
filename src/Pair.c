/* :tabSize=4:indentSize=4:folding=indent: */
#include <stdlib.h>
#include "astring.h"
#include "Logging.h"
#include "Pair.h"

Pair *new_Pair(char *key, void *value){
	Pair *result = NULL;
	
	if((result = (Pair *)malloc(sizeof(Pair))) == NULL){
		Logging_fatalf("%s: Out of memory", __FUNCTION__);
	}
	else{
		result->key   = astrcpy(key);
		result->value = value;
	}
	return result;
}


void delete_Pair(Pair *p, bool freeData){
	if(p != NULL){
		if(p->key != NULL) free(p->key);
		if(freeData && p->value != NULL) free(p->value);
		free(p);
	}
	else{
		Logging_warn("delete_Pair(): NULL argument");
	}
}

