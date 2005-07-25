/* :tabSize=4:indentSize=4:folding=indent: */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "astring.h"
#include "path.h"
#include "Logging.h"
#include "stringext.h"

void walkDir(char *dirName){
	DIR    *d        = NULL;
	char   *fileName = NULL;
	char   *fullPath = NULL;
	struct dirent *e = NULL;
	struct stat    s;
	
	if((d = opendir(dirName)) != NULL){
		while((e = readdir(d)) != NULL){
			fileName = e->d_name;
			if(!strequals(fileName, ".") && !strequals(fileName, "..")){
				fullPath = asprintf("%s/%s", dirName, fileName);
				if(stat(fullPath, &s) == 0){
					if(S_ISDIR(s.st_mode)){
						walkDir(fullPath);
					}
					else{
						printf("%s\n", fullPath);
					}
				}
				else{
					Logging_warnf("%s: Unable to stat() file \"%s\": %s",
							__FUNCTION__, fullPath, strerror(errno));
				}
				if(fullPath != NULL) free(fullPath);
			}
		}
		closedir(d);
	}
	else{
		Logging_errorf("%s: Unable to open directory \"%s\": %s", 
				__FUNCTION__, dirName, strerror(errno));
	}
}


int main(int argc, char *argv[]){
	char *currDir   = NULL;
	
	Logging_setup(argv[0], LOG_TOSTDERR | LOG_LEVELTRACE, NULL);
	if(argc < 2){
		Logging_fatal("Not enough arguments!");
	}
	currDir = getCurrentDirectory();
	Logging_infof("%s: Current directory is \"%s\"", __FUNCTION__, currDir);
	
	walkDir(argv[1]);
	if(currDir  != NULL) free(currDir);
	exit(EXIT_SUCCESS);
}
