/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Func.c,v 1.4 2006/04/13 00:01:51 ken Exp $
 */
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "Func.h"
#include "Logging.h"

static Dict *functionList = NULL;

Dict *getFunctionList(void){
	if(functionList == NULL){
		functionList = new_Dict();
		Dict_put(functionList, "length(", Func_length);
		Dict_put(functionList, "substr(", Func_substr);
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

