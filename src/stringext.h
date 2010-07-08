/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: stringext.h,v 1.6 2010/07/08 22:16:15 ken Exp $
 * stringext - Utility functions for manipulating strings
 */
#ifndef STRINGEXT_H_
#define STRINGEXT_H_
#include "bool.h"
#include <stdlib.h>

bool strxequals(const char *s1, const char *s2);
bool strxequalsi(const char *s1, const char *s2);
bool strxempty(const char *s);
bool strxnullorempty(const char *s);
char *strxupper(char *s);
char *strxlower(char *s);
char *strxreplace(char *s, char c1, char c2);
char *strxfilter(char *s, char *allowedChars, char c);
bool strxpos(const char *s, char c, size_t *pos);
bool strxrpos(const char *s, char c, size_t *pos);
bool strxbegins(const char *s1, const char *s2);
bool strxends(const char *s1, const char *s2);
bool strxcontains(const char *s1, const char *s2);
bool strxcontainsi(const char *s1, const char *s2);

#endif /* STRINGEXT_H_ */
