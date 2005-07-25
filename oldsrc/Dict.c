#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "Pair.h"
#include "Dict.h"

ListNode *dictGet(Dict *d, char *key, size_t *index);

Dict *new_Dict(){
   static const char fnName[] __attribute__((unused)) = "new_Dict()";
   
   return (Dict *)new_List();
}


void delete_Dict(Dict *d, bool freeData){
   static const char fnName[] __attribute__((unused)) = "delete_Dict()";
   
   if(d != NULL){
      while(Dict_length(d) > 0){
         Dict_remove(d, 0, freeData);
      }
      free(d);
   }
}


void *Dict_get(Dict *d, char *key){
   static const char fnName[] __attribute__((unused)) = "Dict_get()";
   ListNode *tt     = NULL;
   Pair     *pp     = NULL;
   void     *retVal = NULL;
   
   if(d == NULL || key == NULL || strlen(key) == 0){
      retVal = NULL;
   }
   else{
      tt = dictGet(d, key, NULL);
      if(tt != NULL){
         pp     = (Pair *)tt->data;
         retVal = pp->data;
      }
   }
   return retVal;
}


bool Dict_exists(Dict *d, char *key){
   static const char fnName[] __attribute__((unused)) = "Dict_exists()";
   return (dictGet(d, key, NULL) != NULL);
}


size_t Dict_length(Dict *d){
   static const char fnName[] __attribute__((unused)) = "Dict_length()";
   return List_length((List *)d);
}


void Dict_put(Dict *d, char *key, void *data){
   static const char fnName[] __attribute__((unused)) = "Dict_put()";
   ListNode *tt = NULL;
   Pair     *pp = NULL;
   
   if((tt = dictGet(d, key, NULL)) != NULL){
      pp = (Pair *)tt->data;
      pp->data = data;
   }
   else{
      pp = new_Pair(key, data);
      List_append((List *)d, pp);
   }
}


void Dict_remove(Dict *d, char *key, bool freeData){
   static const char fnName[] __attribute__((unused)) = "Dict_remove()";
   ListNode *ln   = NULL;
   size_t   index = 0;
   
   if((ln = dictGet(d, key, &index)) != NULL){
      delete_Pair((Pair *)ln->data, freeData);
      List_remove((List *)d, index, false);
   }
}


ListNode *dictGet(Dict *d, char *key, size_t *index){
   ListNode *ii   = NULL;
   size_t   idx   = 0;
   Pair     *pp   = NULL;
   bool     found = false;
   if(d != NULL){
      ii = ((List *)d)->first;
      while(ii != NULL){
         pp = (Pair *)ii->data;
         if(strcmp(pp->key, key) == 0){
            found = true;
            break;
         }
         idx++;
         ii = ii->next;
      }
   }
   if(found){
      if(index != NULL) *index = idx;
      return ii;
   }
   else{
      if(index != NULL) *index = 0;
      return NULL;
   }
}
