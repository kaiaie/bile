/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: memutils.c,v 1.1 2005/07/25 22:41:48 ken Exp $
 */
#include <stdlib.h>
#include "memutils.h"
#include "Logging.h"

/* malloc() or die! */
void *mu__malloc(char *fileName, int lineNo, size_t size){
	void *tmp = NULL;
	
	if((tmp = malloc(size)) == NULL){
		Logging__fatal(fileName, lineNo, "Out of memory!");
	}
	return tmp;
}


void *mu__realloc(char *fileName, int lineNo, void *ptr, size_t size){
	void *tmp = NULL;
	
	if((tmp = realloc(ptr, size)) == NULL){
		Logging__fatal(fileName, lineNo, "Out of memory!");
	}
	return tmp;
}


void mu_free(void *ptr){
	if(ptr != NULL) free(ptr);
}

