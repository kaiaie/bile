/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Buffer.c,v 1.6 2010/08/24 20:49:24 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"
#include "Logging.h"
#include "memutils.h"
#include "stringext.h"

#define DEFAULT_INITIAL_LENGTH 64

/** Creates a new buffer empty buffer with the specified initial size */
Buffer *new_Buffer(size_t initialLength){
	char    *s = NULL;
	Buffer  *b = NULL;
	size_t bufferLength;
	
	bufferLength = (initialLength == 0) ? DEFAULT_INITIAL_LENGTH : initialLength;
	s = (char *)mu_malloc(bufferLength);
	b = (Buffer *)mu_malloc(sizeof(Buffer));
	b->data   = s;
	b->length = bufferLength;
	Buffer_reset(b);
	return b;
}


/** Releases buffer storage */
void delete_Buffer(Buffer *b) {
	if (b != NULL) {
		if (b->data != NULL) {
			mu_free(b->data);
		}
		mu_free(b);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** Clears the contents of the buffer. The buffer's allocated size is not 
*** changed
**/
void Buffer_reset(Buffer *b) {
	char *s = NULL;
	
	if (b != NULL) {
		s    = b->data;
		s[0] = '\0';
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** \brief Appends a null-terminated string to the buffer.
*** If the buffer's currently allocated size is insufficient to store its 
*** current contents and the appended string, it is doubled in size until it 
*** can.
**/
void Buffer_appendString(Buffer *b, const char *s) {
	size_t requiredLength;
	size_t proposedLength;
	
	if (b != NULL) {
		requiredLength = strlen(b->data) + strlen(s) + 1;
		if (requiredLength > b->length) {
			proposedLength = b->length * 2;
			while (proposedLength < requiredLength) {
				proposedLength *= 2;
			}
			b->data   = (char *)mu_realloc(b->data, proposedLength);
			b->length = proposedLength;
		}
		strcat(b->data, s);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** Appends a single character to the buffer */
void Buffer_appendChar(Buffer *b, char ch) {
   char s[2] = {ch, '\0'};
   
   Buffer_appendString(b, s);
}


/** Appends the specified number of characters to the buffer */
void Buffer_appendChars(Buffer *b, const char *s, size_t count) {
	size_t ii;
	char   currChr;
	
	if (b != NULL){
		for (ii = 0; ii < count; ++ii) {
			currChr = s[ii];
			if(currChr == '\0') break;
			Buffer_appendChar(b, currChr);
		}
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** Removes the final character from the buffer */
void Buffer_dropChar(Buffer *b) {
	char *s = NULL;
	
	if (b != NULL) {
		s = b->data;
		if(s != NULL && strlen(s) > 0) {
			s[strlen(s) - 1] = '\0';
		}
		else {
			Logging_warnf("%s(): Empty buffer", __FUNCTION__);
		}
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** Converts the contents of the buffer to uppercase */
void Buffer_toUpperCase(Buffer *b) {
	char *s = NULL;
	
	if (b != NULL) {
		s = b->data;
		strxupper(s);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}


/** Converts the contents of the buffer to lowercase */
void Buffer_toLowerCase(Buffer *b) {
	char *s = NULL;
	
	if (b != NULL) {
		s = b->data;
		strxlower(s);
	}
	else {
		Logging_warnNullArg(__FUNCTION__);
	}
}

