#ifndef _STR_H
#define _STR_H
#include <stddef.h>
#include "bool.h"

#define INVALID_STRING 0

typedef int String;

String new_String(const char *initial);

bool String_appendChar(String s, char ch);
bool String_appendString(String s1, String s2);
bool String_clear(String s1);
bool String_compare(String s1, String s2, int *cmp);
bool String_compareIgnoreCase(String s1, String s2, int *cmp);
bool String_copySubstring(String newStr, String oldStr, size_t startPos, size_t l);
bool String_copyString(String newStr, String oldStr);
bool String_equals(String s1, String s2, bool *identical);
bool String_equalsIgnoreCase(String s1, String s2, bool *identical);
bool String_insertChar(String s, char ch, size_t atPos);
bool String_insertString(String s1, String s2, size_t atPos);
bool String_toUpper(String s);
bool String_toLower(String s);
bool String_trimAll(String s);
bool String_trimLeft(String s);
bool String_trimRight(String s);

bool String_getCharAt(String s, size_t atPos, char *ch);
bool String_getChars(String s, char **ch);
bool String_getIndexOf(String s, char ch, size_t startPos, size_t *atPos);
bool String_getLength(String s, size_t *l);

void delete_String(String s);

#endif /* _STR_H */
