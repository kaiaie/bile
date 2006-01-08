/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Buffer.h,v 1.2 2006/01/08 18:02:53 ken Exp $
 */
#ifndef _BUFFER_H
#define _BUFFER_H
#include <stddef.h>

typedef struct _buffer{
	char   *data;
	size_t length;
} Buffer;

Buffer *new_Buffer(size_t initialLength);
void   delete_Buffer(Buffer *b);

void   Buffer_reset(Buffer *b);
void   Buffer_appendString(Buffer *b, const char *s);
void   Buffer_appendChars(Buffer *b, const char *s, size_t count);
void   Buffer_appendChar(Buffer *b, char ch);
void   Buffer_dropChar(Buffer *b);
void   Buffer_toUpperCase(Buffer *b);
void   Buffer_toLowerCase(Buffer *b);

#endif /* _BUFFER_H */
