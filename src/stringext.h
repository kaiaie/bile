/* :tabSize=4:indentSize=4:folding=indent:
 * stringext - Utility functions for manipulating strings
 */
#ifndef _STRINGEXT_H_
#define _STRINGEXT_H_
#include "bool.h"

bool strequals(const char *s1, const char *s2);
bool strequalsi(const char *s1, const char *s2);
char *strupper(char *s);
char *strlower(char *s);
char *strreplace(char *s, char c1, char c2);
char *strfilter(char *s, char *allowedChars, char c);
bool strpos(char *s, char c, size_t *pos);
bool strrpos(char *s, char c, size_t *pos);

#endif /* _STRINGEXT_H_ */
