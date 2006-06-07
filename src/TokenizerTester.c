/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: TokenizerTester.c,v 1.1 2006/06/07 20:00:23 ken Exp $
 * To build:
 * gcc -Wall -O2 -o TokenizerTester.exe TokenizerTester.c Buffer.c List.c \
 * Logging.c memutils.c stringext.c tokenize.c astring.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"
#include "List.h"
#include "Logging.h"
#include "tokenize.h"

int main(int argc, char *argv[]){
	Buffer *b      = new_Buffer(0);
	List   *tokens = NULL;
	int    currChar;
	size_t ii;
	
	Logging_setup(argv[0], LOG_LEVELWARN, NULL);
	while(true){
		while((currChar = getchar()) != '\n'){
			Buffer_appendChar(b, currChar);
		}
		if(strcasecmp(b->data, "quit") == 0) break;
		tokens = tokenize(b->data);
		for(ii = 0; ii < List_length(tokens); ++ii)
			puts(List_getString(tokens, ii));
		delete_List(tokens, true);
		Buffer_reset(b);
	}
	delete_Buffer(b);
	exit(EXIT_SUCCESS);
}
