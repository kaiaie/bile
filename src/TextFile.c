/* :tabSize=4:indentSize=4:folding=indent: */
#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "TextFile.h"
#include "Logging.h"

TextFile *new_TextFile(const char *fileName){
	TextFile *result = NULL;
	FILE     *f      = NULL;
	Buffer   *b      = NULL;
	
	if((result = (TextFile *)malloc(sizeof(TextFile))) != NULL){
		if((f = fopen(fileName, "r")) != NULL){
			result->f = f;
			b = new_Buffer(132);
			result->b = b;
			result->state = 0;
		}
		if(f == NULL){
			free(result);
			Logging_fatalf("new_TextFile(): Cannot open file \"%s\": %s", 
					fileName, strerror(errno));
		}
	}
	else{
		Logging_fatal("new_TextFile(): Out of memory!");
	}
	return result;
}


void delete_TextFile(TextFile *t){
	if(t != NULL){
		if(t->f != NULL) fclose(t->f);
		if(t->b != NULL) delete_Buffer(t->b);
		free(t);
	}
	else{
		Logging_warn("delete_TextFile(): NULL argument.");
	}
}


const char *TextFile_readLine(TextFile *t){
	int currChr;
	
	if(t != NULL){
		if(t->state == 2 /* EOF */) return (const char *)NULL;
		else{
			Buffer_reset(t->b);
			while(true){
				currChr = fgetc(t->f);
				if(currChr == EOF){
					t->state = 2;
					return (const char *)t->b->data;
				}
				if(t->state == 0){
					if(currChr == '\r'){
						t->state = 1;
						return (const char *)t->b->data;
					}
					else if(currChr == '\n'){
						return (const char *)t->b->data;
					}
					else
						Buffer_appendChar(t->b, currChr);
				}
				else if(t->state == 1){
					/* Check for \n if last character was \r.  This shouldn't 
					 * be necessary as most versions of the standard library 
					 * convert all line endings to \n, but just in case...
					 */
					if(currChr != '\n'){
						Buffer_appendChar(t->b, currChr);
					}
					t->state = 0;
				}
			}
		}
	}
	else{
		Logging_warn("TextFile_readLine(): NULL argument.");
		return (const char *)NULL;
	}
}


void TextFile_rewind(TextFile *t){
	if(t != NULL){
		if(t->f != NULL) rewind(t->f);
	}
	else{
		Logging_warn("TextFile_rewind(): NULL argument.");
	}
}


long TextFile_tell(TextFile *t){
	long result = 0;
	if(t != NULL){
		if(t->f != NULL) result = ftell(t->f);
	}
	else{
		Logging_warn("TextFile_rewind(): NULL argument.");
	}
	return result;
}

