#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "Pair.h"

Pair *new_Pair(char *key, void *data){
   static const char fnName[] __attribute__((unused)) = "new_Pair()";
   Pair *p = NULL;
   
   if((p = (Pair *)malloc(sizeof(Pair))) == NULL){
      fatal(fnName, "Out of memory!");
   }
   p->key  = copyString(key);
   p->data = data;
   return p;
}


void delete_Pair(Pair *p, bool freeData){
   static const char fnName[] __attribute__((unused)) = "delete_Pair()";
   
   if(p != NULL){
      if(p->key != NULL) free(p->key);
      if(freeData && p->data != NULL) free(p->data);
      free(p);
   }
}
