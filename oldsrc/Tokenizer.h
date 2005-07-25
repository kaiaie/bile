#ifndef _TOKENIZER_H
#define _TOKENIZER_H
#include <stddef.h>
#include "bool.h"

typedef struct _tokenizer{
   char   *input;
   size_t currPos;
   char   *currToken;
   bool   hasMoreTokens;
} Tokenizer;

Tokenizer  *new_Tokenizer(char *input);
void       delete_Tokenizer(Tokenizer *t);
void       Tokenizer_nextToken(Tokenizer *t);
bool       Tokenizer_hasMoreTokens(Tokenizer *t);
const char *Tokenizer_getCurrentToken(Tokenizer *t);

#endif /* _TOKENIZER_H */
