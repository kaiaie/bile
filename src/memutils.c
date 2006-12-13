/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: memutils.c,v 1.5 2006/12/13 22:57:57 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include "memutils.h"
#include "Logging.h"

static char buffer[256];

/**
 * \brief Wrapper around malloc(); if malloc() returns NULL, terminate the 
 * program
 */
void *mu__malloc(char *fileName, int lineNo, size_t size){
	void *tmp = NULL;
	
	if((tmp = malloc(size)) == NULL){
		Logging__fatal(fileName, lineNo, "Out of memory!");
	}
	sprintf(buffer, "++++ Allocated pointer 0x%x of %d bytes", (unsigned int)tmp, size);
	Logging__trace(fileName, lineNo, buffer);
	return tmp;
} /* mu__malloc */


/**
 * \brief Wrapper around realloc(); if realloc() returns NULL, terminate 
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
} /* mu__realloc */


/**
 * \brief Check pointer is not NULL before free()'ing it as this can screw up 
 * some impelementations
 */
void mu__free(char *fileName, int lineNo, void *ptr){
	if(ptr != NULL){
		sprintf(buffer, "++++ Freed pointer 0x%x", (unsigned int)ptr);
		Logging__trace(fileName, lineNo, buffer);
		free(ptr);
	}
} /* mu__free */

