/** :tabSize=4:indentSize=4:folding=indent:
*** $Id: Buffer.h,v 1.4 2010/08/24 20:49:24 ken Exp $
*** \file Buffer.h
*** \brief A resizable character buffer
**/
#ifndef _BUFFER_H
#define _BUFFER_H
#include <stddef.h>

typedef struct tag_buffer{
	/** The contents of the buffer */
	char   *data;
	/** The allocated size of the buffer */
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
