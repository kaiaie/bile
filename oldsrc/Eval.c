/* :tabSize=3:indentSize=3: */
#include <string.h>
#include "bool.h"
#include "tokenizer.h"

void expected(char *msg) __attribute__ ((noreturn));

void expected(char *msg){
	fprintf(stderr, "FATAL: Expected %s\n", msg);
	exit(EXIT_FAILURE);
}


bool isBoolean(const char *token){
	return ((strcmpi(token, "true") == 0) || (strcmpi(token, "false") == 0);
}


bool isOrOp(Tokenizer *t){
   return ((strcmpi(Tokenizer_getCurrentToken(t), "or") == 0) ||
         (strcmpi(Tokenizer_getCurrentToken(t), "xor") == 0));
}


bool getBoolean(Tokenizer *t){
   bool retVal = false;
   if(t != NULL){
      if(!isBoolean(Tokenizer_getCurrentToken(t))){
         expected("Boolean literal");
      }
      else{
         retVal = (strcmpi(Tokenizer_getCurrentToken(t)), "true") == 0);
      }
   }
   return retVal;
}


void evaluate(char *expression){
	Tokenizer *t = new Tokenizer(expression);
	if(Tokenizer_hasMoreTokens(t){
	   Tokenizer_getNextToken(t);
      printf("%d\n", getBoolean(t);
	}
}
