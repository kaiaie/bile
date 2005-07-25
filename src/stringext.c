/* :tabSize=4:indentSize=4:folding=indent: */
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include "stringext.h"

bool strequals(const char *s1, const char *s2){
	return (strcmp(s1, s2) == 0);
}


bool strequalsi(const char *s1, const char *s2){
	return (strcmpi(s1, s2) == 0);
}


char *strupper(char *s){
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		s[ii] = toupper(s[ii]);
	return s;
}


char *strlower(char *s){
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		s[ii] = tolower(s[ii]);
	return s;
}


char *strreplace(char *s, char c1, char c2){
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		if(s[ii] == c1) s[ii] = c2;
	return s;
}


char *strfilter(char *s, char *allowedChars, char c){
/* strfilter - replace any characters in s that aren't in allowedChars with 
 * character c.
*/
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		if(strchr(allowedChars, s[ii]) == NULL) s[ii] = c;
	return s;
}


bool strpos(char *s, char c, size_t *pos){
	size_t ii;
	bool found = false;
	
	if(s != NULL){
		for(ii = 0; ii < strlen(s); ++ii){
			if(s[ii] == c){
				*pos  = ii;
				found = true;
				break;
			}
		}
	}
	return found;
}


bool strrpos(char *s, char c, size_t *pos){
	size_t ii;
	bool found = false;
	
	if(s != NULL){
		for(ii = (strlen(s) - 1); ii >= 0; --ii){
			if(s[ii] == c){
				*pos  = ii;
				found = true;
				break;
			}
		}
	}
	return found;
}
