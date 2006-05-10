/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Func.c,v 1.9 2006/05/10 22:33:35 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "astring.h"
#include "Buffer.h"
#include "Func.h"
#include "Logging.h"
#include "memutils.h"
#include "path.h"
#include "Type.h"

static Dict *functionList = NULL;

Dict *getFunctionList(void){
	if(functionList == NULL){
		functionList = new_Dict();
		Dict_put(functionList, "length(", Func_length);
		Dict_put(functionList, "substr(", Func_substr);
		Dict_put(functionList, "now(", Func_now);
		Dict_put(functionList, "strftime(", Func_strftime);
		Dict_put(functionList, "file(", Func_file);
		Dict_put(functionList, "file_exists(", Func_fileExists);
		Dict_put(functionList, "tag(", Func_tag);
		Dict_put(functionList, "ent(", Func_ent);
		Dict_put(functionList, "exec(", Func_exec);
		Dict_put(functionList, "defined(", Func_defined);
	}
	return functionList;
}


/* Func_length: returns the length of a BILE string
 */
char *Func_length(Vars *v, int argc, char *argv[]){
	if(argc != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", argc);
		return astrcpy("0");
	}
	else{
		return asprintf("%d", strlen(argv[0]));
	}
}


/* Func_substr: returns the current date and time
 */
char *Func_now(Vars *v, int argc, char *argv[]){
	if(argc != 0){
		Logging_warn("Function now() takes no arguments.");
	}
	return asprintf("%d", time(NULL));
}


/* Func_substr: returns a substring of a BILE string
 */
char *Func_substr(Vars *v, int argc, char *argv[]){
	size_t start = 0;
	size_t len   = 0;
	if(argc != 2 && argc != 3){
		Logging_warnf("Got %d argument(s). Expected 2 or 3.", argc);
		return astrcpy("");
	}
	else{
		start = atol(argv[1]);
		if(argc == 3) len = atol(argv[2]);
		return astrmid(argv[0], start, len);
	}
}


/* Func_strftime: wrapper around strftime() call
 */
char *Func_strftime(Vars *v, int argc, char *argv[]){
	time_t timeStamp;
	struct tm *timeStruct = NULL;
	size_t bufferSize = 32;
	char *buffer = NULL;
	
	buffer = mu_malloc(bufferSize);
	if(argc != 2){
		Logging_warnf("Got %d argument(s). Expected 2.", argc);
		return astrcpy("");
	}
	else{
		timeStamp = Type_toLong(argv[1]);
		timeStruct = localtime(&timeStamp);
		if(timeStruct == NULL){
			Logging_warnf("Unable to convert timestamp \"%s\" to local time.", argv[1]);
			return astrcpy("");
		}
		while(true){
			if(strftime(buffer, bufferSize, argv[0], timeStruct) > 0) break;
			bufferSize *= 2;
			buffer = mu_realloc(buffer, bufferSize);
		}
		return buffer;
	}
}


/* Func_file: returns the contents of a specified file as as string
 */
char *Func_file(Vars *v, int argc, char *argv[]){
	FILE *f = NULL;
	size_t fileSize;
	char *result = NULL;
	
	if(argc != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", argc);
		return astrcpy("");
	}
	else if(!fileExists(argv[0])){
		Logging_warnf("File \"%s\" doesn't exist.", argv[0]);
		return astrcpy("");
	}
	else if(getFileSize(argv[0]) < 0){
		Logging_warnf("Error getting size of file \"%s\": %s.", 
			argv[0], strerror(errno));
		return astrcpy("");
	}
	else{
		if((f = fopen(argv[0], "rb")) != NULL){
			fileSize = (size_t)getFileSize(argv[0]);
			result = (char *)mu_malloc(fileSize + 1);
			memset(result, 0, fileSize + 1);
			fread(result, sizeof(char), fileSize , f);
			fclose(f);
			return result;
		}
		else{
			Logging_warnf("Error reading file \"%s\": %s.", 
				argv[0], strerror(errno));
			return astrcpy("");
		}
	}
}


/* Func_fileExists: returns True if the named file exists, False otherwise
 */
char *Func_fileExists(Vars *v, int argc, char *argv[]){
	if(argc != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", argc);
		return astrcpy("");
	}
	return fileExists(argv[0]) ? astrcpy("true") : astrcpy("false");
}


/* Func_tag: Returns an SGML element (tag) with optional attributes.
 * NOTE: No checking is done to determine if the element is valid for the 
 * current DOCTYPE.
 */
char *Func_tag(Vars *v, int argc, char *argv[]){
	Buffer *buffer = NULL;
	char *result = NULL;
	size_t ii = 1;
	if((argc % 2) != 1){
		Logging_warnf("tag() takes an odd number of arguments; got %d.", argc);
		return astrcpy("");
	}
	buffer = new_Buffer(0);
	Buffer_appendChar(buffer, '<');
	Buffer_appendString(buffer, argv[0]);
	while(ii < argc){
		Buffer_appendChar(buffer, ' ');
		Buffer_appendString(buffer, argv[ii]);
		Buffer_appendString(buffer, "=\"");
		Buffer_appendString(buffer, argv[ii + 1]);
		Buffer_appendChar(buffer, '"');
		ii += 2;
	}
	Buffer_appendChar(buffer, '>');
	result = astrcpy(buffer->data);
	delete_Buffer(buffer);
	return result;
}


/* Func_ent: Returns an SGML entity reference.
 * NOTE: No checking is done to determine if the entity is defined in the 
 * document's DTD.
 */
char *Func_ent(Vars *v, int argc, char *argv[]){
	if(argc != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", argc);
		return astrcpy("");
	}
	return asprintf("&%s;", argv[0]);
}


/* Func_ent: Runs an external program and captures its output
 */
char *Func_exec(Vars *v, int argc, char *argv[]){
	Buffer *output = NULL;
	int    outputChar;
	FILE   *pipe = NULL;
	char   *result = NULL;
	
	if(argc != 1){
		Logging_warnf("exec() takes a single argument. Got %d.", argc);
		return astrcpy("");
	}
	if((pipe = popen(argv[0], "r")) != NULL){
		output = new_Buffer(0);
		while((outputChar = fgetc(pipe)) != EOF)
			Buffer_appendChar(output, outputChar);
		/* Save return code */
		Vars_set(v, "error", asprintf("%d", pclose(pipe)));
		result = astrcpy(output->data);
		delete_Buffer(output);
		return result;
	}
	else{
		Logging_warnf("Error in exec(): %s.", strerror(errno));
		return astrcpy("");
	}
};


char *Func_defined(Vars *v, int argc, char *argv[]){
	if(argc != 1){
		Logging_warnf("defined() takes a single argument. Got %d.", argc);
		return astrcpy("");
	}
	return (Vars_defined(v, argv[0]) ? astrcpy("true") : astrcpy("false"));
}
