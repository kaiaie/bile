#include <stdio.h>
#include <stdlib.h>
#include "str.h"

int main(int argc, char *argv[]){
   String s1;
   String s2;
   char   *c1 = NULL;
   
   s1 = new_String("This ");
   s2 = new_String("is a test");
   
   if(!String_appendString(s1, s2)){
      fprintf(stderr, "String_appendString(): Uh oh!\n");
   }
   else{
      if(String_getChars(s1, &c1)){
         printf("%s\n", c1);
      }
      else{
         fprintf(stderr, "String_getChars(): Uh oh!\n");
      }
   }
   
   delete_String(s1);
   delete_String(s2);
   
   return EXIT_SUCCESS;
}
