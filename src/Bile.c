/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Bile.c,v 1.1 2006/03/12 01:06:48 ken Exp $
 */
#include <dirent.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "astring.h"
#include "BileObj.h"
#include "bool.h"
#include "Dict.h"
#include "Expr.h"
#include "FileHandler.h"
#include "Func.h"
#include "HtmlHandler.h"
#include "ImgHandler.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "stringext.h"
#include "TextFile.h"
#include "tokenize.h"

char *inputDir    = NULL;
char *outputDir   = NULL;
char *templateDir = NULL;
bool verboseMode  = false;
bool forceMode    = false;

Publication *thePublication = NULL;
Dict *functionTable = NULL;

void checkDir(char *dirPath);
void addDir(Publication *p, Section *s, const char *path);
void readConfig(Publication *p, Section *s, const char *fileName);

int main(int argc, char *argv[]){
	int option;
	
	/* Initialise logging */
	Logging_setup(argv[0], LOG_TOSTDERR | LOG_LEVELTRACE, NULL);
	
	/* Initialise function table */
	/* TODO: Move this to its own file -- and add a few more functions! */
	functionTable = new_Dict();
	Dict_put(functionTable, "length(", (void *)Func_length);
	Dict_put(functionTable, "substr(", (void *)Func_substr);
	
	/* Read command-line args */
	while((option = getopt(argc, argv, "fvi:o:t:")) != -1){
		switch(option){
			case 'v': verboseMode = true; break;
			case 'f': forceMode = true; break;
			case 'i': inputDir = optarg; break;
			case 'o': outputDir = optarg; break;
			case 't': templateDir = optarg; break;
			default:
				Logging_errorf("Unrecognised option: %c", option);
				exit(EXIT_FAILURE);
		}
	}
	Logging_infof("Verbose mode: %s", (verboseMode ? "on" : "off"));
	Logging_infof("Force mode: %s", (forceMode ? "on" : "off"));
	checkDir(inputDir);
	Logging_infof("Input directory: %s", inputDir);
	checkDir(outputDir);
	Logging_infof("Output directory: %s", outputDir);
	checkDir(templateDir);
	Logging_infof("Template directory: %s", templateDir);
	
	/* Create the publication */
	thePublication = new_Publication();
	addDir(thePublication, NULL, inputDir);
	
	/* Generate the publication */
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


/*
 * addDir -- read a directory and add its contents to the publication
 */
void addDir(Publication *p, Section *s, const char *path){
	const char pubConfigFileName[] = "publication.bile";
	const char sectionConfigFileName[] = "section.bile";
	const char *configFileName = NULL;
	DIR *d = NULL;
	struct dirent *e = NULL;
	struct stat st;
	char    *fullName    = NULL;
	Section *currSection = NULL;
	Section *newSection  = NULL;
	Story   *newStory    = NULL;
	
	/* If section is NULL, we're at the top level */
	if(s == NULL){
		configFileName = pubConfigFileName;
		currSection = (Section *)p;
		/* Add a few useful variables to the publication */
		Vars_let(p->variables, "pi", "3.141592653589793");
	}
	else{
		currSection = s;
		configFileName = sectionConfigFileName;
		/* TODO: Figure out what section variables shouldn't be inherited and default them */
	}
	/* Read the config file if it exists */
	fullName = asprintf("%s/%s", path, configFileName);
	if(access(fullName, F_OK | R_OK)){
		readConfig(p, s, fullName);
	}
	else{
		Logging_warnf("Can't find configuration file %s: %s", fullName, strerror(errno));
	}
	mu_free(fullName);
	
	/* Read files */
	if((d = opendir(path)) == NULL)
		Logging_fatalf("Error opening directory %s: %s", path, strerror(errno));
	while((e = readdir(d)) != NULL){
		if(!strequals(e->d_name, ".") && !strequals(e->d_name, "..")){
			fullName = asprintf("%s/%s", path, e->d_name);
			if(stat(fullName, &st) != 0){
				Logging_warnf("Error stat()'ing file %s: %s",
					fullName, strerror(errno)
				);
				continue;
			}
			/* Is the current directory entry a subdirectory? */
			if(S_ISDIR(st.st_mode)){
				/* Create new section */
				newSection = new_Section(currSection);
				List_append(currSection->sections, newSection);
				addDir(p, newSection, fullName);
			}
			/* Skip configuration files */
			else if(!strends(fullName, ".bile")){
				newStory = new_Story(currSection);
				/* Read metadata from file */
				/* TODO: Set up filehandlers properly */
				if(htmlCanHandle(fullName))
					htmlReadMetadata(fullName, newStory->variables);
				else if(imgCanHandle(fullName))
					imgReadMetadata(fullName, newStory->variables);
				defaultReadMetadata(fullName, newStory->variables);
				List_append(currSection->stories, newStory);
				/* TODO: Add file to section and pub indexes */
			}
			mu_free(fullName);
		}
	}
	closedir(d);
}


/*
 * readConfig - read a configuration file and define variables and indexes
 */
void readConfig(Publication *p, Section *s, const char *fileName){
	const char *aLine = NULL;
	TextFile *t = new_TextFile(fileName);
	List     *l = NULL;
	bool     gotIndex = false;
	Section  *currSection = NULL;
	Index    *currIndex   = NULL;
	Vars     *currVars    = NULL;
	size_t   lineNo = 1;
	char     *varName = NULL;
	char     *varValue = NULL;
	Expr     *e = NULL;
	
	if(s == NULL)
		currSection = (Section *)p;
	else
		currSection = s;
	currVars = currSection->variables;
	while((aLine = TextFile_readLine(t)) != NULL){
		if(strempty(aLine) || aLine[0] == '#') continue; /* Skip blank lines and comments */
		l = tokenize(aLine);
		if(strequals((char *)List_get(l, 0), "index")){
			if(gotIndex){
				Logging_warnf("File %s, line %u: Duplicate index declaration", 
					fileName, lineNo
				);
			}
			else{
				gotIndex = true;
				currIndex = new_Index(currSection, (char *)List_get(l, 1));
				List_append(currSection->indexes, currIndex);
				currVars = currIndex->variables;
			}
		}
		else if(strequals((char *)List_get(l, 0), "endindex")){
			if(gotIndex){
				gotIndex = false;
				currVars = currSection->variables;
			}
			else{
				Logging_warnf("File %s, line %u: Unexpected 'end index' encountered", 
					fileName, lineNo
				);
			}
		}
		else{
			varName = (char *)List_get(l, 0);
			/* Looking for lines of the form:
			 *     $varname = expression
			 */
			if(List_length(l) >= 3 && varName[0] == '$' && strequals((char *)List_get(l, 0), "=")){
				varName = astrcpy(&varName[1]);
				/* Remove the variable name and equals sign */
				List_remove(l, 0, true);
				List_remove(l, 0, true);
				/* Evaluate */
				e = new_Expr2(l, currVars, functionTable);
				varValue = Expr_evaluate(e);
				/* Store value */
				Vars_let(currVars, varName, varValue);
				/* Cleanup */
				mu_free(varName);
			}
			else{
				Logging_warnf("File %s, line %u: Syntax error: expected variable declaration", 
					fileName, lineNo
				);
			}
		}
		delete_List(l, true);
		lineNo++;
	}
	delete_TextFile(t);
}
