#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

char *copyString(char *input){
   static const char fnName[] __attribute__((unused)) = "copyString()";
   char *inputCopy = NULL;
   
   if((inputCopy = (char *)malloc((strlen(input) + 1) * sizeof(char))) == NULL){
      fatal(fnName, "Out of memory!");
   }
   else{
      strcpy(inputCopy, input);
   }
   return inputCopy;
}


char *copyStrings(char *str1, char *str2){
   static const char fnName[] __attribute__((unused)) = "copyStrings()";
   char   *retVal   = NULL;
   size_t totalSize = 0;
   
   totalSize = (strlen(str1) + strlen(str2) + 1) * sizeof(char);
   if((retVal = (char *)malloc(totalSize)) == NULL){
      fatal(fnName, "Out of memory!");
   }
   strcpy(retVal, str1);
   strcat(retVal, str2);
   return retVal;
}


bool isWhitespace(char ch){
   static char wsChars[] = "\t\n\r \240";
   
   return (strchr(wsChars, ch) != NULL);
}


bool isAllWhitespace(char *s){
   bool   retVal = true;
   size_t ii;
   
   if(s != NULL && strlen(s) > 0){
      for(ii = 0; ii < strlen(s); ++ii){
         if(!isWhitespace(s[ii])){
            retVal = false;
            break;
         }
      }
   }
   return retVal;
}


char lastCharOf(char *s){
   if(s == NULL || strlen(s) == 0){
      return '\0';
   }
   else{
      return s[strlen(s) - 1];
   }
}


void fatal(const char *location, const char *message){
   fprintf(stderr, "FATAL: %s: %s\n", location, message);
   exit(EXIT_FAILURE);
}
