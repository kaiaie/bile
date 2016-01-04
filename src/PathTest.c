#include <stdio.h>
#include <stdlib.h>
#include "Logging.h"
#include "memutils.h"
#include "path.h"


int main(int argc, char *argv[]){
	char *currDir  = NULL;
	char relPath[] = "../doc";
	char testPart[] = "/foo/bar/test.txt";
	char *filePart = NULL;
	char *newDir   = NULL;
	
	Logging_setup(argv[0], LOG_TOSTDERR | LOG_LEVELTRACE, NULL);
	currDir = getCurrentDirectory();
	Logging_infof("Current directory is: \"%s\"", currDir);
	newDir = getCombinedPath(currDir, relPath);
	Logging_infof("Doc directory is: \"%s\"", newDir);
	filePart = getPathPart(testPart, PATH_FILE);
	Logging_infof("The file part of \"%s\" with extension is: \"%s\"", 
		testPart, filePart);
	mu_free(filePart);
	filePart = getPathPart(testPart, PATH_FILEONLY);
	Logging_infof("The file part of \"%s\" without extension is: \"%s\"", 
		testPart, filePart);
	mu_free(filePart);
	mu_free(newDir);
	mu_free(currDir);
	exit(EXIT_SUCCESS);
}
