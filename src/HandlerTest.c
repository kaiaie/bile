/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: HandlerTest.c,v 1.2 2006/01/08 18:02:53 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include "bool.h"
#include "Dict.h"
#include "FileHandler.h"
#include "HtmlHandler.h"
#include "ImgHandler.h"
#include "Logging.h"

int main(int argc, char *argv[]){
	/* A default file to stop gdb mangling the command line */
	char defaultFileName[] = "C:\\Documents and Settings\\ken\\My Documents\\SITES\\BEER\\SITE\\INDEX.HTM";
	char *fileName = NULL;
	Dict *d  = NULL;
	
	Logging_setup(argv[0], LOG_TOSTDERR | LOG_LEVELTRACE, NULL);
	fileName = (argc == 1) ? defaultFileName : argv[1];
	d = new_Dict();
	if(htmlCanHandle(fileName)){
		htmlReadMetadata(fileName, d);
	}
	else if(imgCanHandle(fileName)){
		imgReadMetadata(fileName, d);
	}
	defaultReadMetadata(fileName, d);
	Dict_dump(d, NULL);
	delete_Dict(d, true);
	exit(EXIT_SUCCESS);
}
