/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Bile.c,v 1.7 2006/05/16 13:30:01 ken Exp $
 */
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "astring.h"
#include "BileObj.h"
#include "bool.h"
#include "Logging.h"
#include "path.h"
#include "memutils.h"
#include "stringext.h"

Publication *thePublication = NULL;

void checkDir(char *dirPath);

int main(int argc, char *argv[]){
	char *inputDir    = NULL;
	char *outputDir   = NULL;
	char *templateDir = NULL;
	bool verboseMode  = false;
	bool forceMode    = false;
	int option;
	char *currDir = getCurrentDirectory();
	char *logFile = NULL;
	unsigned long logFlags = LOG_LEVELWARN;
	
	/* Read command-line args */
	while((option = getopt(argc, argv, "fvi:o:t:l:")) != -1){
		switch(option){
			case 'v': verboseMode = true; break;
			case 'f': forceMode = true; break;
			case 'i': inputDir = getCombinedPath(currDir, optarg); break;
			case 'o': outputDir = getCombinedPath(currDir, optarg); break;
			case 't': templateDir = getCombinedPath(currDir, optarg); break;
			case 'l': logFile = optarg; break;
			default:
				Logging_errorf("Unrecognised option: %c", option);
				exit(EXIT_FAILURE);
		}
	}
	/* Initialise logging */
	if(verboseMode) logFlags |= LOG_LEVELINFO;
	if(logFile != NULL) logFlags |= LOG_TOFILE;
	Logging_setup(argv[0], logFlags, logFile);
	
	Logging_infof("Verbose mode: %s", (verboseMode ? "on" : "off"));
	Logging_infof("Force mode: %s", (forceMode ? "on" : "off"));
	checkDir(inputDir);
	Logging_infof("Input directory: %s", inputDir);
	checkDir(outputDir);
	Logging_infof("Output directory: %s", outputDir);
	checkDir(templateDir);
	Logging_infof("Template directory: %s", templateDir);
	
	/* Create the publication */
	thePublication = new_Publication(inputDir, outputDir, templateDir, 
		forceMode, verboseMode);
	Publication_build(thePublication);
	
	/* Generate the publication */
	Publication_generate(thePublication);
	
	/* Clean up */
	mu_free(templateDir);
	mu_free(outputDir);
	mu_free(inputDir);
	mu_free(currDir);
	exit(EXIT_SUCCESS);
}


void checkDir(char *dirPath){
	/* TODO: Add flags to check both existence and writeability */
	struct stat st;
	if(dirPath == NULL){
		Logging_error("No directory specified");
		exit(EXIT_FAILURE);
	}
	if(stat(dirPath, &st) != 0){
		Logging_errorf("%s: %s", dirPath, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(!S_ISDIR(st.st_mode)){
		Logging_errorf("%s: not a directory", dirPath);
		exit(EXIT_FAILURE);
	}
	if(access(dirPath, R_OK) == -1){
		Logging_errorf("%s: %s", dirPath, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

