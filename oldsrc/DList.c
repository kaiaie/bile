#include <stdio.h>
#include <stdlib.h>
#include "bool.h"
#include "DList.h"

DList *new_DList(){
   static const char fnName[] __attribute__((unused)) = "new_Dlist()";
   DList *dl = NULL;
   
   if((dl = (DList *)malloc(sizeof(DList))) == NULL){
      fatal(fnName, "Out of memory!");
   }
   dl->prev = NULL;
   dl->next = NULL;
   dl->data = NULL;
   return dl;
}


void delete_DList(DList *dl, bool freeData){
   static const char fnName[] __attribute__((unused)) = "delete_DList()";

   if(dl != NULL){
      if(dl->next != NULL){
         delete_DList(dl->next, freeData);
      }
      if(freeData && dl->data != NULL){
         free(dl->data);
      }
      if(dl->prev != NULL){
         (dl->prev)->next = NULL;
      }
      free(dl);
   }
}


size_t DList_length(DList *dl){
   static const char fnName[] __attribute__((unused)) = "DList_length()";
   size_t retVal = 0;
   DList  *iter  = dl;
   
   if(dl != NULL){
      while(iter->next != NULL){
         iter = iter->next;
         retVal++;
      }
   }
   return retVal;
}
