/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: astring.h,v 1.6 2007/07/03 12:00:14 ken Exp $
 */
/** 
 * \file astring.h 
 * \brief Allocated string functions; functions that return newly-allocated 
 * strings rather than modifying their arguments.
 *
 * It is the responsibility of the caller to free() them at the appropriate 
 * time.  Many of these functions have counterparts in the standard C library, 
 * while others are convenience functions.
 */
#ifndef ASTRING_H
#define ASTRING_H
#include <stdarg.h>
#include <stddef.h>

char *astrcpy(const char *src);
char *astrncpy(const char *src, size_t n);
char *astrcat(const char *src1, const char *src2);
char *asprintf(const char *fmt, ...);
char *astrleft(const char *src, size_t count);
char *astrmid(const char *src, size_t offset, size_t count);
char *astrright(const char *src, size_t count);
char *astrfillch(size_t len, char c);
char *astrfill(size_t len, const char *s);
char *astrunquote(const char *s);
char *astrupper(const char *s);
char *astrlower(const char *s);
char *adirname(const char *path);
char *abasename(const char *path);
char **astrtok(const char *s, const char *delims);
size_t alength(char **a);
void astrtokfree(char **l);
char *astrrev(const char *s);
char *astrltrim(const char *s);
char *astrrtrim(const char *s);
char *astrtrim(const char *s);

#endif /* ASTRING_H */
