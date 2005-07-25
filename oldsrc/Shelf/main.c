#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bool.h"
#include "cfgfile.h"
#include "gc.h"

#define DEFAULT_FILE_NAME "config.bile"

bool verboseMode = false;
bool forceMode   = false;
bool debugMode   = false;

void usage(void);

int main(int argc, char *argv[]){
	int  i;
	char *fileName = NULL;
	char defaultFileName[] = DEFAULT_FILE_NAME;

   /* TODO: Write proper getopt() command-line parser */
	for(i = 1; i < argc; ++i){
		if(strcmp(argv[i], "-v") == 0){
		   verboseMode = true;
		}
		else if(strcmp(argv[i], "-f") == 0){
		   forceMode = true;
		}
		else if(strcmp(argv[i], "-d") == 0){
		   debugMode = true;
		}
		else if(fileName == NULL){
		   fileName = argv[i];
		}
	} /* for */
	
	if(fileName == NULL){
	   /* Use default filename */
	   fileName = defaultFileName;
	}
		
	return EXIT_SUCCESS;
}


void usage(void){
	printf("\n");
}

