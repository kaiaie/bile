/* :tabSize=4:indentSize=4:
 * $Id: astring.c,v 1.12 2010/07/08 22:16:15 ken Exp $
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "bool.h"
#include "Logging.h"
#include "memutils.h"
#include "stringext.h"

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

/**
 * Returns a copy of the string passed to it
 *
 * (equivalent to strdup() found in some standard libraries)
 */
char *astrcpy(const char *src){
	char *inputCopy = NULL;
	
	if(src == NULL){
		Logging_warnf("%s(): Trying to copy NULL string!", __FUNCTION__);
		inputCopy = astrcpy("");
	}
	else{
		inputCopy = (char *)mu_malloc((strlen(src) + 1) * sizeof(char));
		strcpy(inputCopy, src);
	}
	return inputCopy;
}


char *astrcat(const char *src1, const char *src2){
	/* Returns a new string comprising of the two strings passed */
	/* to it joined together.                                    */
	char   *catString = NULL;
	size_t catLength  = strlen(src1) + strlen(src2) + 1;
	
	if(src1 == NULL){
		Logging_warnf("%s(): First argument is NULL!", __FUNCTION__);
		catString = astrcpy(src2);
	}
	else if(src2 == NULL){
		Logging_warnf("%s(): Second argument is NULL!", __FUNCTION__);
		catString = astrcpy(src1);
	}
	else{
		catString = (char *)mu_malloc(catLength * sizeof(char));
		strcpy(catString, src1);
		strcat(catString, src2);
	}
	return catString;
}


char *astrleft(const char *src, size_t count){
	/* Returns a new string consisting of the leftmost count          */
	/* characters of the source string; equivalent to BASIC           */
	/* Left$() function                                               */
	char   *leftString = NULL;
	size_t actualCount;
	
	if(count == 0){
		return astrcpy("");
	}
	else if(count >= strlen(src)){
		return astrcpy(src);
	}
	else{
		leftString = (char *)mu_malloc((count + 1) * sizeof(char));
		if(count >= strlen(src)){
			actualCount = strlen(src);
		}
		else{
			actualCount = count;
		}
		memcpy(leftString, src, actualCount);
		leftString[actualCount] = '\0';
	}
	return leftString;
}


char *astrmid(const char *src, size_t offset, size_t count){
	/* Returns a new string consisting of the count characters        */
	/* starting at offset in the source string.                       */
	/* If count is zero, all characters to the end of the string are  */
	/* returned.  Equivalent to BASIC Mid$() function.                */
	char *midString = NULL;
	
	size_t actualCount;
	if(offset >= strlen(src)){
		return NULL;
	}
	else{
		if((count == 0) || (offset + count > strlen(src))){
			actualCount = strlen(src) - offset;
		}
		else{
			actualCount = count;
		}
		midString = (char *)mu_malloc((actualCount + 1) * sizeof(char));
		memcpy(midString, &src[offset], actualCount);
		midString[actualCount] = '\0';
	}
	return midString;
}


char *astrright(const char *src, size_t count){
	/* Returns a new string consisting of the rightmost count         */
	/* characters of the source string.  Equivalent to BASIC Right$() */
	if(count >= strlen(src)){
		return astrcpy(src);
	}
	else{
		return astrcpy(&src[strlen(src) - count]);
	}
}


char *asprintf(const char *fmt, ...){
	size_t bufferLength = 64;
	size_t newLength;
	char *buffer = NULL;
	va_list ap;
	
	buffer = (char *)mu_malloc(bufferLength * sizeof(char));
	va_start(ap, fmt);
	do{
		if(vsnprintf(buffer, bufferLength, fmt, ap) >= bufferLength){
			newLength = bufferLength * 2;
			buffer = (char *)mu_realloc(buffer, newLength * sizeof(char));
			bufferLength = newLength;
		}
		else break;
	} while(true);
	va_end(ap);
	return buffer;
}


char *astrfillch(size_t len, char c){
	/* Returns a string consisting of len occurrences of c. */
	char *buffer = NULL;
	size_t ii;
	
	buffer = (char *)mu_malloc((len + 1) * sizeof(char));
	for(ii = 0; ii < len; ++ii){
		buffer[ii] = c;
		buffer[ii + 1] = '\0';
	}
	return buffer;
}


char *astrfill(size_t len, const char *s){
	char   *result = NULL;
	size_t ii, jj = 0;
	
	if(strlen(s) <= 0){
		Logging_fatalf("%s: Illegal argument.", __FUNCTION__);
	}
	else if(strlen(s) == 1){
		result = astrfillch(len, s[0]);
	}
	else{
		result = (char *)mu_malloc((len + 1) * sizeof(char));
		for(ii = 0; ii < len; ++ii){
			result[ii] = s[jj];
			result[ii + 1] = '\0';
			jj = (jj + 1) % strlen(s);
		}
	}
	return result;
}


char *astrunquote(const char *s){
	/* Returns a string consisting of a copy of s with the first and last 
	 * characters removed.  Does not actually check if said characters are 
	 * quotes before doing so!
	 */
	if(strlen(s) == 2) return astrcpy("");
	else return astrmid(s, 1, strlen(s) - 2);
}


char *astrupper(const char *s){
	/* Returns a string consisting of a copy of s with all characters converted 
	 * to upper case.
	 */
	return strxupper(astrcpy(s));
}


char *astrlower(const char *s){
	/* Returns a string consisting of a copy of s with all characters converted 
	 * to lower case.
	 */
	return strxlower(astrcpy(s));
}


char *adirname(const char *path){
	/* Returns a string consisting of a copy of the directory part of path.  
	 * Does not change path.  Backslashes are converted into forward slashes.
	 * A version of this function exists in libiberty but the doc is very 
	 * ambiguous about when and if it alters its input.
	 */
	char *result = NULL;
	char *tmp    = NULL;
	size_t ii;
	
	if(strxnullorempty(path)){
		result = astrcpy(".");
	}
	else{
		tmp = strxreplace(astrcpy(path), '\\', '/');
		if(strchr(tmp, '/') == NULL){
			result = astrcpy(".");
		}
		else if(strxequals(tmp, "/") || strxequals(tmp, ".") || strxequals(tmp, "..")){
			result = astrcpy(tmp);
		}
		else{
			if(tmp[strlen(tmp) - 1] == '/') tmp[strlen(tmp) - 1] = '\0';
		}
		ii = strlen(tmp);
		while(ii >= 0){
			if(tmp[ii] == '/'){
				if(ii == 0){
					result = astrcpy("/");
				}
				else{
					result = astrleft(tmp, ii - 1);
				}
				break;
			}
			ii--;
		}
	}
	mu_free(tmp);
	return result;
}


char *abasename(const char *path){
	/* Returns a string consisting of a copy of the file part of path.  
	 * Does not change path.  Backslashes are converted into forward slashes.
	 */
	char *result = NULL;
	char *tmp    = NULL;
	size_t ii;
	
	if(strxnullorempty(path)){
		result = astrcpy(".");
	}
	else{
		tmp = strxreplace(astrcpy(path), '\\', '/');
		if(strchr(tmp, '/') == NULL){
			result = astrcpy(path);
		}
		else if(strxequals(tmp, "/") || strxequals(tmp, ".") || strxequals(tmp, "..")){
			result = astrcpy(tmp);
		}
		else{
			if(tmp[strlen(tmp) - 1] == '/') tmp[strlen(tmp) - 1] = '\0';
			ii = strlen(tmp);
			while(ii >= 0){
				if(tmp[ii] == '/'){
					result = astrmid(tmp, ii + 1, 0);
					break;
				}
				ii--;
			}
		}
	}
	mu_free(tmp);
	return result;
}


char **astrtok(const char *s, const char *delims){
	/* Breaks the string s where it encounters any of the characters in delims.
	 * Returns a NULL-terminated array of strings.
	 */
	char   **result = NULL;
	size_t ii, jj;
	size_t last;
	size_t count = 1;
	size_t len;
	
	if(s != NULL && strlen(s) > 0){
		for(ii = 0; ii < strlen(s); ++ii){
			if(strchr(delims, s[ii]) != NULL)
				count++;
		}
		result = (char**)mu_malloc((count + 1) * sizeof(char *));
		result[count] = NULL;
		if(count == 1){
			result[0] = astrcpy(s);
		}
		else{
			jj = 0;
			last = 0;
			for(ii = 0; ii < strlen(s); ++ii){
				if(strchr(delims, s[ii]) != NULL){
					if(ii == 0){
						result[jj++] = astrcpy("");
						last = 1;
					}
					else{
						len = ii - last;
						if(len > 0)
							result[jj++] = astrmid(s, last ,len);
						else
							result[jj++] = astrcpy("");
						last = ii + 1;
						if(ii == strlen(s) - 1){
							result[jj++] = astrcpy("");
						}
					}
				}
			}
			if(strchr(delims, s[strlen(s) - 1]) == NULL){
				result[jj++] = astrmid(s, last, 0);
			}
		}
	}
	return result;
}


/* alength: Returns the length of a NULL-terminated string */
size_t alength(char **a){
	size_t result = 0;
	while(a[result] != NULL) result++;
	return result;
}


void astrtokfree(char **l){
	/* Frees the strings and array returned by astrtok()
	 */
	size_t ii = 0;
	
	if(l != NULL){
		while(l[ii] != NULL) mu_free(l[ii++]);
		mu_free(l);
	}
	else{
		Logging_warnf("%s(): NULL argument", __FUNCTION__);
	}
}


char *astrrev(const char *s){
	char *result = NULL;
	size_t ii;
	
	result = (char *)malloc((strlen(s) + 1) * sizeof(char));
	result[strlen(s)] = '\0';
	for(ii = 0; ii < strlen(s); ++ii){
		result[ii] = s[strlen(s) - ii - 1];
	}
	return result;
}


char *astrltrim(const char *s){
	char   *result = NULL;
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii){
		if(!isspace(s[ii])){
			result = astrmid(s, ii, 0);
			break;
		}
	}
	if(result == NULL) result = astrcpy("");
	return result;
}


char *astrrtrim(const char *s){
	char   *result = NULL;
	size_t ii;
	
	for(ii = strlen(s) - 1; ii >= 0; --ii){
		if(!isspace(s[ii])){
			result = astrleft(s, ii + 1);
			break;
		}
	}
	if(result == NULL) result = astrcpy("");
	return result;
}


char *astrtrim(const char *s){
	char *tmp1 = NULL;
	char *tmp2 = NULL;
	
	tmp1 = astrltrim(s);
	tmp2 = astrrtrim(tmp1);
	mu_free(tmp1);
	return tmp2;
}


char *astrncpy(const char *src, size_t n){
	char *result = NULL;
	
	if(strlen(src) <= n){
		return astrcpy(src);
	}
	else{
		result = (char *)mu_malloc((n + 1) * sizeof(char));
		strncpy(result, src, n);
		return result;
	}
}
