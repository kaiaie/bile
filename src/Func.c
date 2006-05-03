/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Func.c,v 1.5 2006/05/03 10:14:10 ken Exp $
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "astring.h"
#include "Func.h"
#include "Logging.h"
#include "memutils.h"
#include "Type.h"

static Dict *functionList = NULL;

Dict *getFunctionList(void){
	if(functionList == NULL){
		functionList = new_Dict();
		Dict_put(functionList, "length(", Func_length);
		Dict_put(functionList, "substr(", Func_substr);
		Dict_put(functionList, "strftime(", Func_strftime);
	}
	return functionList;
}

char *Func_length(int argc, char *argv[]){
	if(argc != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", argc);
		return astrcpy("0");
	}
	else{
		return asprintf("%d", strlen(argv[0]));
	}
}


char *Func_substr(int argc, char *argv[]){
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


char *Func_strftime(int argc, char *argv[]){
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
