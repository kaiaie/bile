/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: stringext.c,v 1.5 2006/06/05 17:15:24 ken Exp $
 */
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include "stringext.h"

bool strequals(const char *s1, const char *s2){
	return (strcmp(s1, s2) == 0);
}


bool strequalsi(const char *s1, const char *s2){
	return (strcasecmp(s1, s2) == 0);
}


bool strempty(const char *s){
	return (strlen(s) == 0);
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
		ii = strlen(s) - 1;
		while(true){
			if(s[ii] == c){
				*pos  = ii;
				found = true;
				break;
			}
			if(ii == 0) break;
			ii--;
		}
	}
	return found;
}


bool strbegins(char *s1, char *s2){
	return (strncmp(s1, s2, strlen(s2)) == 0);
}


bool strends(char *s1, char *s2){
	if(strlen(s1) < strlen(s2))
		return false;
	else
		return (strcmp(&s1[strlen(s1) - strlen(s2)], s2) == 0);
}

