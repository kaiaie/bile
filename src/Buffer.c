/* :tabSize=4:indentSize=4:folding=indent: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"
#include "Logging.h"
#include "stringext.h"

#define DEFAULT_INITIAL_LENGTH 64

Buffer *new_Buffer(size_t initialLength){
	char    *s = NULL;
	Buffer  *b = NULL;
	size_t bufferLength;
	
	bufferLength = (initialLength == 0) ? DEFAULT_INITIAL_LENGTH : initialLength;
	s = (char *)malloc(bufferLength);
	b = (Buffer *)malloc(sizeof(Buffer));
	if(s == NULL || b == NULL){
		Logging_fatalf("%s(): Out of memory!", __FUNCTION__);
	}
	b->data   = s;
	b->length = bufferLength;
	Buffer_reset(b);
	return b;
}


void delete_Buffer(Buffer *b){
   if(b != NULL){
      if(b->data != NULL){
         free(b->data);
      }
      free(b);
   }
   else{
	   Logging_warnf("%s(): NULL argument", __FUNCTION__);
   }
}


void Buffer_reset(Buffer *b){
   char *s = NULL;
   
   if(b != NULL){
      s    = b->data;
      s[0] = '\0';
   }
   else{
	   Logging_warnf("%s(): NULL argument", __FUNCTION__);
   }
}


void Buffer_appendString(Buffer *b, const char *s){
   size_t requiredLength;
   size_t proposedLength;
   
   if(b != NULL){
      requiredLength = strlen(b->data) + strlen(s) + 1;
      if(requiredLength > b->length){
         proposedLength = b->length * 2;
         while(proposedLength < requiredLength){
            proposedLength *= 2;
         }
         if((b->data = (char *)realloc(b->data, proposedLength)) == NULL){
            Logging_fatalf("%s(): Unable to increase buffer size.", __FUNCTION__);
         }
         b->length = proposedLength;
      }
      strcat(b->data, s);
   }
   else{
	   Logging_warnf("%s(): NULL argument", __FUNCTION__);
   }
}


void Buffer_appendChar(Buffer *b, char ch){
   char s[2] = {ch, '\0'};
   
   Buffer_appendString(b, s);
}


void Buffer_appendChars(Buffer *b, const char *s, size_t count){
	size_t ii;
	char   currChr;
	
	for(ii = 0; ii < count; ++ii){
		currChr = s[ii];
		if(currChr == '\0') break;
		Buffer_appendChar(b, currChr);
	}
}


void Buffer_dropChar(Buffer *b){
	/* Drop the final character from the buffer */
	char *s = NULL;
	
	if(b != NULL){
		s = b->data;
		if(s != NULL && strlen(s) > 0){
			s[strlen(s) - 1] = '\0';
		}
		else{
			Logging_warnf("%s(): Empty buffer", __FUNCTION__);
		}
	}
	else{
	   Logging_warnf("%s(): NULL argument", __FUNCTION__);
	}
}


void Buffer_toUpperCase(Buffer *b){
	char *s     = NULL;
	if(b != NULL){
		s = b->data;
		strupper(s);
	}
	else{
	   Logging_warnf("%s(): NULL argument", __FUNCTION__);
	}
}


void Buffer_toLowerCase(Buffer *b){
	char *s     = NULL;
	if(b != NULL){
		s = b->data;
		strlower(s);
	}
	else{
	   Logging_warnf("%s(): NULL argument", __FUNCTION__);
	}
}
