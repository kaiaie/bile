#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "bool.h"
#include "Buffer.h"
#include "Logging.h"
#include "memutils.h"

char *htmltok(FILE *htmlFile){
	enum {INITIAL, READ_TAG, READ_TEXT};
	static Buffer *b = NULL;
	int currChar;
	int currState    = INITIAL;
	bool keepGoing   = true;
	bool keepChar    = true;
	char *result     = NULL;
	
	if(b == NULL) b = new_Buffer(0);
	Buffer_reset(b);
	if(!feof(htmlFile)){
		while(keepGoing){
			currChar = fgetc(htmlFile);
			if(currChar != EOF){
				if(currState == INITIAL)
					currState = (currChar == '<') ? READ_TAG : READ_TEXT;
				else if(currState == READ_TAG){
					if(currChar == '>')
						keepGoing = false;
				}
				else if(currState == READ_TEXT){
					if(currChar == '<'){
						ungetc(currChar, htmlFile);
						keepChar  = false;
						keepGoing = false;
					}
				}
				if(keepChar)
					Buffer_appendChar(b, currChar);
				else
					keepChar = true;
			}
			else
				keepGoing = false;
		}
		result = astrcpy(b->data);
	}
	else{
		/* DONE */
		delete_Buffer(b);
	}
	return result;
}


int main(int argc, char *argv[]){
	FILE *f = NULL;
	char *token = NULL;
	Logging_setup(argv[0], LOG_TOSTDERR | LOG_LEVELTRACE, NULL);
	if((f = fopen(argv[1], "r")) != NULL){
		while((token = htmltok(f)) != NULL){
			printf("Got token: \"%s\"\n", token);
			mu_free(token);
		}
		fclose(f);
		exit(EXIT_SUCCESS);
	}
	else{
		Logging_errorf("Unable to open file \"%s\": %s", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
}
