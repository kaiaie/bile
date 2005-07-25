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
void   Buffer_appendChar(Buffer *b, char ch);

#endif
