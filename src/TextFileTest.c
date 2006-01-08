/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: TextFileTest.c,v 1.2 2006/01/08 18:02:54 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include "Logging.h"
#include "TextFile.h"

int main(int argc, char *argv[]){
	TextFile *t = NULL;
	const char *line = NULL;
	
	Logging_setup(argv[0], LOG_LEVELDEBUG | LOG_TOSTDERR, NULL);
	t = new_TextFile(argv[1]);
	while((line = TextFile_readLine(t)) != NULL){
		printf("%s\n", line);
	}
	delete_TextFile(t);
	exit(EXIT_SUCCESS);
}
