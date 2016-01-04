#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include "stringext.h"
#include "memutils.h"

/** Convenience function: Returns True if s1 and s2 are identical */
bool strxequals(const char *s1, const char *s2){
	return (strcmp(s1, s2) == 0);
}


/** Convenience function: Returns True if s1 and s2 are identical (case-insensitive) */
bool strxequalsi(const char *s1, const char *s2){
	return (strcasecmp(s1, s2) == 0);
}


/** Convenience function: Returns True if s is a zero-length string */
bool strxempty(const char *s){
	return (strlen(s) == 0);
}


/** Convenience function: Returns True if s is NULL or a zero-length string */
bool strxnullorempty(const char *s) {
	return ((s == NULL) || (strlen(s) == 0));
}


/** Uppercases a string in place */
char *strxupper(char *s){
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		s[ii] = toupper(s[ii]);
	return s;
}


/** Lowercases a string in place */
char *strxlower(char *s){
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		s[ii] = tolower(s[ii]);
	return s;
}


/** Replaces all instances of character c1 in s with character c2 */
char *strxreplace(char *s, char c1, char c2){
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		if(s[ii] == c1) s[ii] = c2;
	return s;
}


/** Replaces any characters in s that aren't in allowedChars with character c. */
char *strxfilter(char *s, char *allowedChars, char c){
	size_t ii;
	
	for(ii = 0; ii < strlen(s); ++ii)
		if(strchr(allowedChars, s[ii]) == NULL) s[ii] = c;
	return s;
}


/** Returns True if character c is found in string s; the offset is returned in pos */
bool strxpos(const char *s, char c, size_t *pos){
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


/** Returns True if character c is found in string s; the offset is returned in pos. 
*** Similar to strxpos except that the search is performed from the end of the 
*** string rather than from the beginning
***/
bool strxrpos(const char *s, char c, size_t *pos){
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


/** Returns True if string s1 begins with s2 */
bool strxbegins(const char *s1, const char *s2){
	return (strncmp(s1, s2, strlen(s2)) == 0);
}


/** Returns True if string s1 ends with s2 */
bool strxends(const char *s1, const char *s2){
	if(strlen(s1) < strlen(s2))
		return false;
	else
		return (strcmp(&s1[strlen(s1) - strlen(s2)], s2) == 0);
}


/** Convenience function: Returns True if s2 is contained in s1 */
bool strxcontains(const char *s1, const char *s2){
	return (strstr(s1, s2) != NULL);
}


/** Convenience function: Returns True if s2 is contained in s1 (case-insensitive) */
bool strxcontainsi(const char *s1, const char *s2){
	bool result = false;
	char *t1 = NULL;
	char *t2 = NULL;
	
	t1 = (char *)mu_malloc((strlen(s1) + 1) * sizeof(char));
	strcpy(t1, s1);
	strxupper(t1);
	t2 = (char *)mu_malloc((strlen(s2) + 1) * sizeof(char));
	strcpy(t2, s2);
	strxupper(t2);
	result = strxcontains(t1, t2);
	mu_free(t2);
	mu_free(t1);
	return result;
}

