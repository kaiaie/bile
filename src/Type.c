/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Type.c,v 1.2 2006/01/08 18:02:54 ken Exp $
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "Type.h"

bool Type_isNumeric(char *val){
	bool retVal = true;
	bool gotDot = false;
	int  ii = 0;
	
	for(ii = 0; ii < strlen(val); ++ii){
		if(!isdigit(val[ii])){
			if(val[ii] == '.'){
				if(gotDot){
					retVal = false;
				}
				else{
					gotDot = true;
				}
			}
			else{
				retVal = false;
			}
			if(!retVal) break;
		}
	}
	return retVal;
}


bool Type_isDouble(char *val){
	/* Returns True if the argument is numeric and contains a decimal point */
	return (Type_isNumeric(val) && (strchr(val, '.') != NULL));
}


bool Type_isLong(char *val){
	/* Returns True if the argument is numeric and doesn't contain a decimal point */
	return (Type_isNumeric(val) && (strchr(val, '.') == NULL));
}


bool Type_toBool(char *val){
	/* Convert string to boolean value.  If the string is empty, the
	 * literal string "false", or it can be converted to the number zero,
	 * return False; otherwise True.
	 */
	if(strlen(val) == 0){
		return false;
	}
	else if(strcmpi(val, "false") == 0){
		return false;
	}
	else if(atol(val) == 0){
		return false;
	}
	else{
		return true;
	}
}


long Type_toLong(char *val){
	return atol(val);
}


double Type_toDouble(char *val){
	return atof(val);
}
