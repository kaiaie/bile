/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Type.c,v 1.7 2010/08/31 15:11:58 ken Exp $
*/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "Type.h"
#include "stringext.h"

/** Returns True if supplied character value is numeric */
bool Type_isNumeric(char *val){
	bool retVal = true;
	bool gotDot = false;
	int  ii = 0;
	
	for (ii = 0; ii < strlen(val); ++ii) {
		if (!isdigit(val[ii])) {
			if (ii == 0 && val[ii] == '-') continue;
			if (val[ii] == '.') {
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


/** Returns True if the argument is numeric and contains a decimal point */
bool Type_isDouble(char *val){
	return (Type_isNumeric(val) && (strchr(val, '.') != NULL));
}


/** Returns True if the argument is numeric and doesn't contain a decimal point */
bool Type_isLong(char *val){
	return (Type_isNumeric(val) && (strchr(val, '.') == NULL));
}

/** \brief Converts a string to a boolean value. 
*** 
*** A string evaluates to False if:
*** <ul>
*** <li>the string is empty, </li>
*** <li>the literal string "false", or </li> 
*** <li>it can be converted to the number zero,</li>
*** </ul>
*** All other strings evaluate to True.
**/
bool Type_toBool(char *val){
	if (strlen(val) == 0) {
		return false;
	}
	else if (strxequalsi(val, "false")) {
		return false;
	}
	else if (Type_isNumeric(val) && atol(val) == 0) {
		return false;
	}
	else {
		return true;
	}
}


/** \brief Converts a string to a long value. */
long Type_toLong(char *val){
	return atol(val);
}


/** \brief Converts a string to a double value. */
double Type_toDouble(char *val){
	return atof(val);
}
