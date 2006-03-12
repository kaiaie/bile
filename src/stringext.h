/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: stringext.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 * stringext - Utility functions for manipulating strings
 */
#ifndef STRINGEXT_H_
#define STRINGEXT_H_
#include "bool.h"

bool strequals(const char *s1, const char *s2);
bool strequalsi(const char *s1, const char *s2);
bool strempty(const char *s);
char *strupper(char *s);
char *strlower(char *s);
char *strreplace(char *s, char c1, char c2);
char *strfilter(char *s, char *allowedChars, char c);
bool strpos(char *s, char c, size_t *pos);
bool strrpos(char *s, char c, size_t *pos);
bool strbegins(char *s1, char *s2);
bool strends(char *s1, char *s2);

#endif /* STRINGEXT_H_ */
