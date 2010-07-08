/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: stringext.h,v 1.5 2010/07/08 21:04:25 ken Exp $
 * stringext - Utility functions for manipulating strings
 */
#ifndef STRINGEXT_H_
#define STRINGEXT_H_
#include "bool.h"
#include <stdlib.h>

bool strequals(const char *s1, const char *s2);
bool strequalsi(const char *s1, const char *s2);
bool strempty(const char *s);
char *strupper(char *s);
char *strlower(char *s);
char *strreplace(char *s, char c1, char c2);
char *strfilter(char *s, char *allowedChars, char c);
bool strpos(const char *s, char c, size_t *pos);
bool strrpos(const char *s, char c, size_t *pos);
bool strbegins(const char *s1, const char *s2);
bool strends(const char *s1, const char *s2);
bool strcontains(const char *s1, const char *s2);
bool strcontainsi(const char *s1, const char *s2);

#endif /* STRINGEXT_H_ */
