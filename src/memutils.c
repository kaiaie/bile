/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: memutils.c,v 1.2 2006/05/10 09:54:34 ken Exp $
 */
#include <stdlib.h>
#include "memutils.h"
#include "Logging.h"

/* mu__malloc: Wrapper around malloc(); if malloc() returns NULL, terminate 
 * the program
 */
void *mu__malloc(char *fileName, int lineNo, size_t size){
	void *tmp = NULL;
	
	if((tmp = malloc(size)) == NULL){
		Logging__fatal(fileName, lineNo, "Out of memory!");
	}
	return tmp;
}


/* mu__realloc: Wrapper around realloc(); if realloc() returns NULL, terminate 
 * the program
 */
void *mu__realloc(char *fileName, int lineNo, void *ptr, size_t size){
	void *tmp = NULL;
	
	if((tmp = realloc(ptr, size)) == NULL){
		Logging__fatal(fileName, lineNo, "Out of memory!");
	}
	return tmp;
}


/* mu_free: Check pointer is not NULL before free()'ing it (this can screw up 
 * many impelementations)
 */
void mu_free(void *ptr){
	if(ptr != NULL) free(ptr);
}

