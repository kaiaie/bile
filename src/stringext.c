/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: stringext.c,v 1.7 2010/07/08 21:04:25 ken Exp $
 */
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include "stringext.h"
#include "memutils.h"

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


bool strpos(const char *s, char c, size_t *pos){
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


bool strrpos(const char *s, char c, size_t *pos){
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


bool strbegins(const char *s1, const char *s2){
	return (strncmp(s1, s2, strlen(s2)) == 0);
}


bool strends(const char *s1, const char *s2){
	if(strlen(s1) < strlen(s2))
		return false;
	else
		return (strcmp(&s1[strlen(s1) - strlen(s2)], s2) == 0);
}


bool strcontains(const char *s1, const char *s2){
	return (strstr(s1, s2) != NULL);
}


bool strcontainsi(const char *s1, const char *s2){
	bool result = false;
	char *t1 = NULL;
	char *t2 = NULL;
	
	t1 = (char *)mu_malloc((strlen(s1) + 1) * sizeof(char));
	strcpy(t1, s1);
	strupper(t1);
	t2 = (char *)mu_malloc((strlen(s2) + 1) * sizeof(char));
	strcpy(t2, s2);
	strupper(t2);
	result = strcontains(t1, t2);
	mu_free(t2);
	mu_free(t1);
	return result;
}

