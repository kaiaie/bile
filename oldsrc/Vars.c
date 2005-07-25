#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "Pair.h"
#include "Vars.h"

void *varsGet(Vars *v, char *name, bool stringVar);
void varsPut(Vars *v, char *name, void *value, bool stringVar);


Vars *new_Vars(){
   static const char fnName[] __attribute__((unused)) = "new_Vars()";
   
   return (Vars *)new_Dict();
}


void delete_Vars(Vars *v){
   static const char fnName[] __attribute__((unused)) = "delete_Vars()";
   delete_Dict((Dict *)v, true); /* FIXME: This will leak */
}

size_t Vars_length(Vars *v){
   static const char fnName[] __attribute__((unused)) = "Vars_length()";
   return Dict_length((Dict *)v);
}


bool Vars_exists(Vars *v, char *name){
   static const char fnName[] __attribute__((unused)) = "Vars_exists()";
   return Dict_exists((Dict *)v, name);
}


char *Vars_getString(Vars *v, char *name){
   static const char fnName[] __attribute__((unused)) = "Vars_getString()";
   
   return (char *)varsGet(v, name, true);
}


void *Vars_getData(Vars *v, char *name){
   static const char fnName[] __attribute__((unused)) = "Vars_getData()";
   return varsGet(v, name, false);
}


void Vars_putString(Vars *v, char *name, char *value){
   static const char fnName[] __attribute__((unused)) = "Vars_putString()";
   varsPut(v, name, value, true);
}


void Vars_putData(Vars *v, char *name, void *data){
   static const char fnName[] __attribute__((unused)) = "Vars_putData()";
   varsPut(v, name, data, false);   
}


void Vars_dump(Vars *v){
   static const char fnName[] __attribute__((unused)) = "Vars_dump()";
   ListNode *ln = NULL;
   Pair     *pp = NULL;
   
   if(v != NULL){
      ln = ((List *)v)->first;
      while(ln != NULL){
         pp = (Pair *)ln->data;
         if(pp->key[0] == '$'){
            printf("%s = \"%s\"\n", pp->key, (char *)pp->data);
         }
         else{
            printf("%s = data\n", pp->key);
         }
         ln = ln->next;
      }
   }
}


void *varsGet(Vars *v, char *name, bool stringVar){
   static const char fnName[] __attribute__((unused)) = "varsGet()";
   char *fullName = NULL;
   void *retVal = NULL;
   
   if(v != NULL){
      if(stringVar){
         fullName = copyStrings("$", name);
      }
      else{
         fullName = copyStrings("@", name);
      }
      retVal = Dict_get((Dict *)v, fullName);
      free(fullName);
   }
   return retVal;
}


void varsPut(Vars *v, char *name, void *value, bool stringVar){
   static const char fnName[] __attribute__((unused)) = "varsPut()";
   char *fullName = NULL;
   void *data     = NULL;
   
   if(v != NULL){
      if(stringVar){
         fullName = copyStrings("$", name);
         if(Vars_exists(v, fullName)){
            Dict_remove((Dict *)v, fullName, true);
         }
         data = copyString((char *)value);
      }
      else{
         fullName = copyStrings("@", name);
         data     = value;
      }
      Dict_put((Dict *)v, fullName, data);
      free(fullName);
   }
}
