/* :tabSize=4:indentSize=4:folding=indent: */
#include <stdio.h>
#include <stdlib.h>
#include "Logging.h"
#include "memutils.h"
#include "path.h"


int main(int argc, char *argv[]){
	char *currDir  = NULL;
	char relPath[] = "../doc";
	char *newDir   = NULL;
	
	Logging_setup(argv[0], LOG_TOSTDERR | LOG_LEVELTRACE, NULL);
	currDir = getCurrentDirectory();
	Logging_infof("Current directory is: \"%s\"", currDir);
	newDir = getCombinedPath(currDir, relPath);
	Logging_infof("Doc directory is: \"%s\"", newDir);
	mu_free(newDir);
	mu_free(currDir);
	exit(EXIT_SUCCESS);
}
