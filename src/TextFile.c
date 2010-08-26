/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: TextFile.c,v 1.6 2010/08/26 09:35:51 ken Exp $
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "TextFile.h"
#include "Logging.h"
#include "memutils.h"

/** Allocates and initialises a new TextFile structure */
TextFile *new_TextFile(const char *fileName){
	TextFile *result = NULL;
	FILE     *f      = NULL;
	Buffer   *b      = NULL;
	
	result = (TextFile *)mu_malloc(sizeof(TextFile));
	if ((f = fopen(fileName, "rb")) != NULL) {
		result->f = f;
		b = new_Buffer(132);
		result->b = b;
		result->state = 0;
	}
	if (f == NULL) {
		mu_free(result);
		Logging_fatalf("%s: Cannot open file \"%s\": %s", 
				__FUNCTION__, fileName, strerror(errno));
	}
	return result;
}


/** Deallocates a TextFile structure, closing any resources */
void delete_TextFile(TextFile *t){
	if (t != NULL){
		if (t->f != NULL) fclose(t->f);
		if (t->b != NULL) delete_Buffer(t->b);
		mu_free(t);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** \brief Reads the next line from the text file 
***
*** \return The next line of text, or NULL if the end of the file has been 
*** reached
**/
const char *TextFile_readLine(TextFile *t){
	int currChr;
	
	if(t != NULL) {
		if (t->state == 2) {
			/* EOF */
			return (const char *)NULL;
		}
		else {
			Buffer_reset(t->b);
			while (true) {
				currChr = fgetc(t->f);
				if (currChr == EOF) {
					t->state = 2;
					return (const char *)t->b->data;
				}
				if (t->state == 0) {
					if (currChr == '\r') {
						t->state = 1;
						return (const char *)t->b->data;
					}
					else if (currChr == '\n') {
						return (const char *)t->b->data;
					}
					else {
						Buffer_appendChar(t->b, currChr);
					}
				}
				else if (t->state == 1) {
					if (currChr != '\n') {
						Buffer_appendChar(t->b, currChr);
					}
					t->state = 0;
				}
			}
		}
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
		return (const char *)NULL;
	}
}


/** Moves the read position back to the start of the file */
void TextFile_rewind(TextFile *t){
	if (t != NULL) {
		if (t->f != NULL) {
			rewind(t->f);
			t->state = 0;
		}
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** Returns the position in the file from where the next read will take place */
long TextFile_tell(TextFile *t) {
	long result = 0;
	if (t != NULL) {
		if (t->f != NULL) result = ftell(t->f);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
	return result;
}

