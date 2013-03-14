/* :tabSize=4:indentSize=4:folding=indent:
** $Id: ConfigFile.c,v 1.3 2010/08/31 15:11:56 ken Exp $
*/
#include "ConfigFile.h"
#include "bool.h"
#include "astring.h"
#include "memutils.h"
#include "stringext.h"
#include "tokenize.h"
#include "Expr.h"
#include "Index.h"
#include "List.h"
#include "Logging.h"
#include "Tags.h"
#include "TextFile.h"
#include "Vars.h"
 
/** Reads a configuration file and defines variables and indexes */
void readConfigFile(Publication *p, Section *s, const char *fileName) {
	const char *aLine = NULL;
	TextFile    *t = new_TextFile(fileName);
	List        *l = NULL;
	bool        gotIndex = false;
	bool        gotTags  = false;
	BileObject	*currObj = (BileObject *)s;
	Index       *currIndex   = NULL;
	Tags        *currTags    = NULL;
	size_t      lineNo = 0;
	char        *varName = NULL;
	char        *varValue = NULL;
	size_t      ii;
	
	Logging_debugf("Reading configuration file %s", fileName);
	while ((aLine = TextFile_readLine(t)) != NULL) {
		lineNo++;
		if (strxempty(aLine) || aLine[0] == '#') continue; /* Skip blank lines and comments */
		l = tokenize(aLine);
		if (List_length(l) == 0){
			Logging_warnf("File %s, line %u: Parse error", fileName, lineNo);
			continue;
		}
		if (strxequals((char *)List_get(l, 0), "index")){
			if (gotIndex || gotTags) {
				Logging_warnf("File %s, line %u: Duplicate index/tag declaration", 
					fileName, lineNo
				);
			} else {
				gotIndex = true;
				currIndex = new_Index(s, (char *)List_get(l, 1));
				List_append(s->indexes, currIndex);
				currObj = (BileObject *)currIndex;
			}
		} else if (strxequals((char *)List_get(l, 0), "endindex")) {
			if (gotIndex) {
				gotIndex = false;
				Logging_debugf("Index variables:");
				currObj = (BileObject *)s;
				Vars_dump(currObj->variables);
			} else {
				Logging_warnf("File %s, line %u: Unexpected 'end index' encountered", 
					fileName, lineNo
				);
			}
		} else if(strxequals((char *)List_get(l, 0), "tags")) {
			if (gotIndex || gotTags) {
				Logging_warnf("File %s, line %u: Missing 'end index'", 
					fileName, lineNo
				);
			} else if (s != p->root) {
				Logging_warnf("File %s, line %u: Tags only allowed in publication configuration file", 
					fileName, lineNo
				);
			} else{
				gotTags = true;
				currTags = new_Tags(p, (char *)List_get(l, 1));
				List_append(p->tagList, currTags);
				currObj = (BileObject *)currTags;
			}
		}
		else if (strxequals((char *)List_get(l, 0), "endtags")) {
			if (gotTags) {
				gotIndex = false;
				Logging_debugf("Tag variables:");
				currObj = (BileObject *)s;
				Vars_dump(currObj->variables);
			} else {
				Logging_warnf("File %s, line %u: Unexpected 'end tags' encountered", 
					fileName, lineNo
				);
			}
		} else {
			varName = (char *)List_get(l, 0);
			/* Looking for lines of the form:
			 *     $varname = expression
			 */
			if (List_length(l) >= 3 && varName[0] == '$' && strxequals(List_getString(l, 1), "=")) {
				varName = astrcpy(&varName[1]);
				/* Remove the variable name and equals sign */
				List_remove(l, 0, true);
				List_remove(l, 0, true);
				/* Evaluate and store */
				varValue = evaluateTokens(l, currObj);
				Vars_let(currObj->variables, varName, varValue, VAR_STD);
				mu_free(varName);
				mu_free(varValue);
			} else {
				Logging_warnf("File %s, line %u: Syntax error: expected variable declaration", 
					fileName, lineNo
				);
			}
		}
		delete_List(l, true);
	}
	delete_TextFile(t);
	Logging_debug("Section variables:");
	Vars_dump(currObj->variables);
	Logging_debug("Section indexes:");
	for (ii = 0; ii < List_length(s->indexes); ++ii) {
		Index_dump((Index *)List_get(s->indexes, ii));
	}
}


