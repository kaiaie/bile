/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Bile.c,v 1.17 2010/08/31 15:53:13 ken Exp $
*/
/**
*** \file Bile.c
*** \brief Program entry point
**/
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "astring.h"
#include "Publication.h"
#include "bool.h"
#include "Logging.h"
#include "path.h"
#include "memutils.h"
#include "stringext.h"
#include "version.h"

static char const rcsId[] = "$Id: Bile.c,v 1.17 2010/08/31 15:53:13 ken Exp $";

Publication *thePublication = NULL;

void checkDir(const char *dirPath);
void usage(const char *appName);

int main(int argc, char *argv[]){
	char *inputDir    = NULL;
	char *outputDir   = NULL;
	char *templateDir = NULL;
	char *scriptFile  = NULL;
	bool verboseMode  = false;
	bool forceMode    = false;
	int option;
	char *currDir = getCurrentDirectory();
	char *logFile = NULL;
	unsigned long logFlags = LOG_LEVELWARN | LOG_TOSTDERR;
	
	/* Display usage */
	if (argc == 1 || strxequalsi(argv[1], "/?") || strxequalsi(argv[1], "-?")) {
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}
	/* Read command-line args */
	while ((option = getopt(argc, argv, "fvi:o:t:l:s:")) != -1){
		switch (option){
			case 'v': verboseMode = true; break;
			case 'f': forceMode = true; break;
			case 'i': inputDir = getCombinedPath(currDir, optarg); break;
			case 'o': outputDir = getCombinedPath(currDir, optarg); break;
			case 't': templateDir = getCombinedPath(currDir, optarg); break;
			case 'l': logFile = optarg; break;
			case 's': scriptFile = optarg; break;
			default:
				usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	/* Initialise logging */
	if (verboseMode) logFlags |= LOG_LEVELDEBUG;
	if (logFile != NULL) logFlags |= LOG_TOFILE;
	Logging_setup(argv[0], logFlags, logFile);
	
	Logging_infof("BILE build: %s", rcsId);
	Logging_infof("Verbose mode: %s", (verboseMode ? "on" : "off"));
	Logging_infof("Force mode: %s", (forceMode ? "on" : "off"));
	checkDir(inputDir);
	Logging_infof("Input directory: %s", inputDir);
	checkDir(outputDir);
	Logging_infof("Output directory: %s", outputDir);
	checkDir(templateDir);
	Logging_infof("Template directory: %s", templateDir);
	if (!strxnullorempty(scriptFile)) {
		Logging_infof("Script file: %s", scriptFile);
	}
	
	/* Create the publication */
	thePublication = new_Publication(inputDir, outputDir, templateDir, 
		forceMode, verboseMode, scriptFile);
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


void checkDir(const char *dirPath){
	/* TODO: Add flags to check both existence and writeability */
	struct stat st;
	if (strxnullorempty(dirPath)) {
		Logging_error("No directory specified");
		exit(EXIT_FAILURE);
	}
	if (stat(dirPath, &st) != 0){
		Logging_errorf("%s: %s", dirPath, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (!S_ISDIR(st.st_mode)){
		Logging_errorf("%s: not a directory", dirPath);
		exit(EXIT_FAILURE);
	}
	if (access(dirPath, R_OK) == -1){
		Logging_errorf("%s: %s", dirPath, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void usage(const char *appName) {
	printf("%s - Basic InLinEr, version %d.%d.%d\n\n", 
		appName, 
		APP_VERSION_MAJOR, 
		APP_VERSION_MINOR,
		APP_VERSION_BUILD
	);
	printf("Usage: %s -i input -o output -t template [-l log] [-s script] [-f] [-v]\n", appName);
	printf("\t-i: Directory where input files are located\n");
	printf("\t-o: Directory where output files are written\n");
	printf("\t-t: Directory where template files are located\n");
	printf("\t-l: Log file\n");
	printf("\t-s: Script file for upload via FTP\n");
	printf("\t-f: Force all output to be rebuilt irrespective of modification date\n");
	printf("\t-v: Verbose mode\n");
}


