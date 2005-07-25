#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

Buffer *new_Buffer(size_t initialLength){
   char   *s = NULL;
   Buffer *b = NULL;
   
   s = (char *)malloc(initialLength);
   b = (Buffer *)malloc(sizeof(Buffer));
   if(s == NULL || b == NULL){
      fprintf(stderr, "FATAL: new_Buffer(): Out of memory!\n");
      exit(EXIT_FAILURE);
   }
   b->data   = s;
   b->length = initialLength;
   Buffer_reset(b);
   return b;
}


void delete_Buffer(Buffer *b){
   if(b != NULL){
      if(b->data != NULL){
         free(b->data);
      }
      free(b);
   }
}


void Buffer_reset(Buffer *b){
   char *s = NULL;
   
   if(b != NULL){
      s    = b->data;
      s[0] = '\0';
   }
}


void Buffer_appendString(Buffer *b, const char *s){
   size_t requiredLength;
   size_t proposedLength;
   
   if(b != NULL){
      requiredLength = strlen(b->data) + strlen(s) + 1;
      if(requiredLength > b->length){
         proposedLength = b->length * 2;
         while(proposedLength < requiredLength){
            proposedLength *= 2;
         }
         if((b->data = (char *)realloc(b->data, proposedLength)) == NULL){
            fprintf(stderr, "FATAL: Buffer_appendString(): Out of memory!\n");
            exit(EXIT_FAILURE);
         }
         b->length = proposedLength;
      }
      strcat(b->data, s);
   }
}


void Buffer_appendChar(Buffer *b, char ch){
   char s[2] = {ch, '\0'};
   
   Buffer_appendString(b, s);
}

