/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Func.c,v 1.17 2009/10/25 13:59:08 ken Exp $
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "astring.h"
#include "BileObj.h"
#include "Buffer.h"
#include "Func.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "Ops.h"
#include "path.h"
#include "Type.h"

extern Publication *thePublication;

static Dict *functionList = NULL;

typedef enum {INDEX_FIRST, INDEX_PREV, INDEX_NEXT, INDEX_LAST} IndexItem;

char *indexItem(Vars *v, List *args, IndexItem what){
	Index *theIndex    = NULL;
	Story *theStory    = NULL;
	char  *varName     = NULL;
	char  defaultVar[] = "file_name";
	bool   found       = false;
	size_t ii;
	
	if(List_length(args) < 1){
		Logging_warn("index_*() takes at least 1 argument.");
		return astrcpy("");
	}
	if(List_length(args) == 1) varName = defaultVar; else varName = List_getString(args, 1);
	if((theIndex = Publication_findIndex(thePublication, List_getString(args, 0))) != NULL){
		switch(what){
			case INDEX_FIRST:
				theStory = (Story *)List_get(theIndex->stories, 0);
				found = true;
				break;
			case INDEX_LAST:
				theStory = (Story *)List_get(theIndex->stories, -1);
				found = true;
				break;
			case INDEX_PREV:
			case INDEX_NEXT:
				for(ii = 0; ii < List_length(theIndex->stories); ++ii){
					theStory = (Story *)List_get(theIndex->stories, ii);
					if(theStory->variables == v){
						found = true;
						break;
					}
				}
				if(found){
					if(what == INDEX_PREV){
						if(ii == 0)
							found = false;
						else
							theStory = (Story *)List_get(theIndex->stories, ii - 1);
					}
					else if(what == INDEX_NEXT){
						if(ii == List_length(theIndex->stories) - 1)
							found = false;
						else
							theStory = (Story *)List_get(theIndex->stories, ii + 1);							
					}
				}
				break;
			default:
				Logging_fatal("Can't happen!");
		}
		if(found)
			return astrcpy(Vars_get(theStory->variables, varName));
		else
			return astrcpy("");
	}
	else{
		Logging_warnf("Could not find index named \"%s\"", List_getString(args, 0));
		return astrcpy("");
	}
}

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
		Dict_put(functionList, "basename(", Func_basename);
		Dict_put(functionList, "dirname(", Func_dirname);
		Dict_put(functionList, "relative_path(", Func_relativePath);
		Dict_put(functionList, "index_first(", Func_indexFirst);
		Dict_put(functionList, "index_prev(", Func_indexPrev);
		Dict_put(functionList, "index_next(", Func_indexNext);
		Dict_put(functionList, "index_last(", Func_indexLast);
		Dict_put(functionList, "decode(", Func_decode);
		Dict_put(functionList, "ucase(", Func_ucase);
		Dict_put(functionList, "lcase(", Func_lcase);
		Dict_put(functionList, "iif(", Func_iif);
		Dict_put(functionList, "indexof(", Func_indexof);
	}
	return functionList;
}


/* Func_length: returns the length of a BILE string
 */
char *Func_length(Vars *v, List *args){
	if(List_length(args) != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", List_length(args));
		return astrcpy("0");
	}
	else{
		return asprintf("%d", strlen(List_getString(args, 0)));
	}
}


/* Func_substr: returns the current date and time
 */
char *Func_now(Vars *v, List *args){
	if(List_length(args) != 0){
		Logging_warn("Function now() takes no arguments.");
	}
	return asprintf("%d", time(NULL));
}


/* Func_substr: returns a substring of a BILE string
 */
char *Func_substr(Vars *v, List * args){
	size_t start = 0;
	size_t len   = 0;
	if(List_length(args) != 2 && List_length(args) != 3){
		Logging_warnf("Got %d argument(s). Expected 2 or 3.", List_length(args));
		return astrcpy("");
	}
	else{
		start = atol(List_getString(args, 1));
		if(List_length(args) == 3) len = atol(List_getString(args, 2));
		return astrmid(List_getString(args, 0), start, len);
	}
}


/* Func_strftime: wrapper around strftime() call
 */
char *Func_strftime(Vars *v, List *args){
	time_t timeStamp;
	struct tm *timeStruct = NULL;
	size_t bufferSize = 32;
	char *buffer = NULL;
	
	buffer = mu_malloc(bufferSize);
	if(List_length(args) != 2){
		Logging_warnf("Got %d argument(s). Expected 2.", List_length(args));
		return astrcpy("");
	}
	else{
		timeStamp = Type_toLong(List_getString(args, 1));
		timeStruct = localtime(&timeStamp);
		if(timeStruct == NULL){
			Logging_warnf("Unable to convert timestamp \"%s\" to local time.", List_getString(args, 1));
			return astrcpy("");
		}
		while(true){
			if(strftime(buffer, bufferSize, List_getString(args, 0), timeStruct) > 0) break;
			bufferSize *= 2;
			buffer = mu_realloc(buffer, bufferSize);
		}
		return buffer;
	}
}


/* Func_file: returns the contents of a specified file as as string
 */
char *Func_file(Vars *v, List *args){
	FILE *f = NULL;
	size_t fileSize;
	char *result = NULL;
	
	if(List_length(args) != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", List_length(args));
		return astrcpy("");
	}
	else if(!fileExists(List_getString(args, 0))){
		Logging_warnf("File \"%s\" doesn't exist.", List_getString(args, 0));
		return astrcpy("");
	}
	else if(getFileSize(List_getString(args, 0)) < 0){
		Logging_warnf("Error getting size of file \"%s\": %s.", 
			List_getString(args, 0), strerror(errno));
		return astrcpy("");
	}
	else{
		if((f = fopen(List_getString(args, 0), "rb")) != NULL){
			fileSize = (size_t)getFileSize(List_getString(args, 0));
			result = (char *)mu_malloc(fileSize + 1);
			memset(result, 0, fileSize + 1);
			fread(result, sizeof(char), fileSize , f);
			fclose(f);
			return result;
		}
		else{
			Logging_warnf("Error reading file \"%s\": %s.", 
				List_getString(args, 0), strerror(errno));
			return astrcpy("");
		}
	}
}


/* Func_fileExists: returns True if the named file exists, False otherwise
 */
char *Func_fileExists(Vars *v, List *args){
	if(List_length(args) != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", List_length(args));
		return astrcpy("");
	}
	return fileExists(List_getString(args, 0)) ? astrcpy("true") : astrcpy("false");
}


/* Func_tag: Returns an SGML element (tag) with optional attributes.
 * NOTE: No checking is done to determine if the element is valid for the 
 * current DOCTYPE.
 */
char *Func_tag(Vars *v, List * args){
	Buffer *buffer = NULL;
	char *result = NULL;
	size_t ii = 1;
	if((List_length(args) % 2) != 1){
		Logging_warnf("tag() takes an odd number of arguments; got %d.", List_length(args));
		return astrcpy("");
	}
	buffer = new_Buffer(0);
	Buffer_appendChar(buffer, '<');
	Buffer_appendString(buffer, List_getString(args, 0));
	while(ii < List_length(args)){
		Buffer_appendChar(buffer, ' ');
		Buffer_appendString(buffer, List_getString(args, ii));
		Buffer_appendString(buffer, "=\"");
		Buffer_appendString(buffer, List_getString(args, ii + 1));
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
char *Func_ent(Vars *v, List *args){
	if(List_length(args) != 1){
		Logging_warnf("Got %d argument(s). Expected 1.", List_length(args));
		return astrcpy("");
	}
	return asprintf("&%s;", List_getString(args, 0));
}


/* Func_ent: Runs an external program and captures its output
 */
char *Func_exec(Vars *v, List *args){
	Buffer *output = NULL;
	int    outputChar;
	FILE   *pipe = NULL;
	char   *result = NULL;
	char   *exitCode = NULL;
	
	if(List_length(args) != 1){
		Logging_warnf("exec() takes a single argument. Got %d.", List_length(args));
		return astrcpy("");
	}
	if((pipe = popen(List_getString(args, 0), "r")) != NULL){
		output = new_Buffer(0);
		while((outputChar = fgetc(pipe)) != EOF)
			Buffer_appendChar(output, outputChar);
		/* Save return code */
		exitCode = asprintf("%d", pclose(pipe));
		Vars_set(v, "error", exitCode, VAR_NOSHADOW);
		mu_free(exitCode);
		result = astrcpy(output->data);
		delete_Buffer(output);
		return result;
	}
	else{
		Logging_warnf("Error in exec(): %s.", strerror(errno));
		return astrcpy("");
	}
};


char *Func_defined(Vars *v, List *args){
	if(List_length(args) != 1){
		Logging_warnf("defined() takes a single argument. Got %d.", List_length(args));
		return astrcpy("");
	}
	return (Vars_defined(v, List_getString(args, 0)) ? astrcpy("true") : astrcpy("false"));
}


char *Func_basename(Vars *v, List *args){
	if(List_length(args) != 1){
		Logging_warnf("defined() takes a single argument. Got %d.", List_length(args));
		return astrcpy("");
	}
	return getPathPart(List_getString(args, 0), PATH_FILE);
}


char *Func_dirname(Vars *v, List *args){
	if(List_length(args) != 1){
		Logging_warnf("defined() takes a single argument. Got %d.", List_length(args));
		return astrcpy("");
	}
	return getPathPart(List_getString(args, 0), PATH_DIR);
}


char *Func_relativePath(Vars *v, List *args){
	if(List_length(args) != 2){
		Logging_warnf("relative_path() takes 2 arguments. Got %d.", List_length(args));
	}
	return getRelativePath(List_getString(args, 0), List_getString(args, 1));
}


char *Func_decode(Vars *v, List *args){
	char *expr = NULL;
	size_t ii;
	if(List_length(args) < 2 || (List_length(args) % 2 != 0)){
		Logging_warnf("decode() takes an even number of arguments. Got %d.", List_length(args));
		return astrcpy("");
	}
	expr = List_getString(args, 0);
	for(ii = 1; ii < List_length(args) - 1; ii += 2){
		if(Op_eq(expr, List_getString(args, ii))){
			return astrcpy(List_getString(args, ii + 1));
		}
	}
	/* No match; return the last argument as default */
	return astrcpy(List_getString(args, List_length(args) - 1));
}


char *Func_ucase(Vars *v, List *args){
	if(List_length(args)  != 1){
		Logging_warnf("ucase() takes a single argument. Got %d.", List_length(args));
	}
	if(List_length(args) == 0){
		return astrcpy("");
	}
	else{
		return astrupper(List_getString(args, 0));
	}
}


char *Func_lcase(Vars *v, List *args){
	if(List_length(args)  != 1){
		Logging_warnf("lcase() takes a single argument. Got %d.", List_length(args));
	}
	if(List_length(args) == 0){
		return astrcpy("");
	}
	else{
		return astrlower(List_getString(args, 0));
	}
}


char *Func_iif(Vars *v, List *args){
	if(List_length(args)  != 3){
		Logging_warnf("lcase() takes 3 arguments. Got %d.", List_length(args));
		return astrcpy("");
	}
	if(Type_toBool(List_get(args, 0))){
		return astrcpy(List_getString(args, 1));
	}
	else{
		return astrcpy(List_getString(args, 2));
	}
}


/* Functions for accessing story variables via an index (for creating Prev/Next
 * links)
 */
char *Func_indexFirst(Vars *v, List *args){
	return indexItem(v, args, INDEX_FIRST);
}


char *Func_indexPrev(Vars *v, List *args){
	return indexItem(v, args, INDEX_PREV);
}


char *Func_indexNext(Vars *v, List *args){
	return indexItem(v, args, INDEX_NEXT);
}


char *Func_indexLast(Vars *v, List *args){
	return indexItem(v, args, INDEX_LAST);
}


char *Func_indexof(Vars *v, List *args)
{
	if ((List_length(args) != 2) && (List_length(args) != 3)){
		Logging_warnf("indexof() takes 2 or 3 arguments. Got %d.", List_length(args));
	}
	else {
		char *str = List_getString(args, 0);
		size_t strLength = strlen(str);
		size_t ii = 0;
		char *s = List_getString(args, 1);
		char ch = s[0];
		if (List_length(args) == 3 && Type_isNumeric(List_getString(args, 2))){
			ii = Type_toLong(List_getString(args, 2));
			if (ii < 0) ii = 0;
		}
		while (ii < strLength) {
			if (str[ii] == ch){
				return asprintf("%ld", ii);
			}
			++ii;
		}
	}
	return astrcpy("-1");
}
