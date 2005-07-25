#include <stdio.h>
#include <stdlib.h>
#include "Tokenizer.h"

int main(int argc, char *argv[]){
   char   test[] = "substr(($myvar . \" test\"), $x, 1)";
   long   tokenCount = 0;

   Tokenizer *t = new_Tokenizer(test);
   
   /* Test harness */
   while(Tokenizer_hasMoreTokens(t)){
      Tokenizer_nextToken(t);
      fprintf(stderr, "%lu: \"%s\"\n", ++tokenCount, 
            Tokenizer_getCurrentToken(t));
   }
   delete_Tokenizer(t);
   
   exit(EXIT_SUCCESS);
}
