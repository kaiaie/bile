#ifndef _UTIL_H
#define _UTIL_H
#include "bool.h"

char *copyString(char *input);
char *copyStrings(char *str1, char *str2);
bool isWhitespace(char ch);
bool isAllWhitespace(char *s);
char lastCharOf(char *s);
void fatal(const char *location, const char *message) __attribute__ ((noreturn));

#endif
