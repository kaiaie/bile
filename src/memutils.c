/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: memutils.c,v 1.3 2006/05/15 09:35:26 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include "memutils.h"
#include "Logging.h"

static char buffer[256];

/* mu__malloc: Wrapper around malloc(); if malloc() returns NULL, terminate 
 * the program
 */
void *mu__malloc(char *fileName, int lineNo, size_t size){
	void *tmp = NULL;
	
	if((tmp = malloc(size)) == NULL){
		Logging__fatal(fileName, lineNo, "Out of memory!");
	}
	sprintf(buffer, "++++ Allocated pointer 0x%x of %d bytes", (unsigned int)tmp, size);
	Logging__trace(fileName, lineNo, buffer);
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
	sprintf(buffer, "++++ Reallocated pointer 0x%x to %d bytes", (unsigned int)tmp, size);
	Logging__trace(fileName, lineNo, buffer);
	return tmp;
}


/* mu_free: Check pointer is not NULL before free()'ing it (this can screw up 
 * many impelementations)
 */
void mu__free(char *fileName, int lineNo, void *ptr){
	if(ptr != NULL){
		sprintf(buffer, "++++ Freed pointer 0x%x", (unsigned int)ptr);
		Logging__trace(fileName, lineNo, buffer);
		free(ptr);
	}
	else{
		Logging__warn(fileName, lineNo, "Tried to free NULL pointer");
	}
}

