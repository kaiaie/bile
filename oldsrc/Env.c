#include <stdio.h>
#include <stdlib.h>
#include "bool.h"
#include "Env.h"
#include "util.h"
#include "VarList.h"

void *envGet(Env *origin, Env *curr, char *key, bool createEmpty);


Env *new_Env(Env *parent){
   static const char fnName[] __attribute__((unused)) = "new_Env()";
   Env     *e   = NULL;
   VarList *vl  = NULL;
   Env     *sib = NULL;
   
   e  = (Env *)malloc(sizeof(Env));
   vl = new_VarList();
   if(e == NULL || vl == NULL){
      fprintf(stderr, "FATAL: %s: Out of memory!\n", fnName);
      exit(EXIT_FAILURE);
   }
   e->parent = parent; /* Join to parent (bottom-up) */
   e->data   = vl;
   e->left   = NULL;
   e->right  = NULL;
   if(e->parent != NULL){
      /* Join to siblings (top-down) */
      if(e->parent->right == NULL){
         e->parent->right = e;
      }
      else{
         sib = e->parent->right;
         while(sib->left != NULL){
            sib = sib->left;
         }
         sib->left = e;
      }
   }
   return e;
}


void *Env_get(Env *e, char *key){
   static const char fnName[] __attribute__((unused)) = "Env_get()";

   return envGet(e, e, key, false);
}


char *Env_getString(Env *e, char *key){
   static const char fnName[] __attribute__((unused)) = "Env_get()";

   return (char *)envGet(e, e, key, true);
}


void Env_set(Env *e, char *key, void *data){
   static const char fnName[] __attribute__((unused)) = "Env_set()";
   VarList *vl = NULL;

   if(e != NULL){
      if(e->data == NULL){
         vl = new_VarList();
         e->data = vl;
      }
      else{
         vl = (VarList*)e->data;
      }
      VarList_set(vl, key, data);
   }
}


void Env_setString(Env *e, char *key, char *data){
   static const char fnName[] __attribute__((unused)) = "Env_setString()";
   VarList *vl = NULL;

   if(e != NULL){
      if(e->data == NULL){
         vl = new_VarList();
         e->data = vl;
      }
      else{
         vl = (VarList*)e->data;
      }
      VarList_setString(vl, key, data);
   }
}


void *envGet(Env *origin, Env *curr, char *key, bool createEmpty){
   static const char fnName[] __attribute__((unused)) = "envGet()";
   VarList *vl   = NULL;
   bool    found = false;
   void    *data = NULL;
   
   if(curr != NULL){
      if(curr->data != NULL){
         vl = (VarList*)curr->data;
         if(VarList_exists(vl, key)){
            data  = VarList_get(vl, key);
            found = true;
         }
      }
      if(!found){
         /* Search in parent */
         if(curr->parent != NULL){
            data = envGet(origin, curr->parent, key, createEmpty);
         }
         else{
            /* Already at the top */
            /* See if there is an environment string with the same name */
            if(key[0] == '$') key++;
            if(getenv(key) != NULL){
               /* Create a copy of the environment string in the original context */
               if(origin != NULL){
                  if(origin->data == NULL){
                     origin->data = new_VarList();
                  }
                  VarList_setString((VarList *)origin->data, key, getenv(key));
                  data = VarList_get((VarList *)origin->data, key);
               }
            }
            else{
               /* Search exhausted */
               if(createEmpty){
                  if(origin != NULL){
                     if(origin->data == NULL){
                        origin->data = new_VarList();
                     }
                     VarList_setString((VarList *)origin->data, key, "");
                     data = VarList_get((VarList *)origin->data, key);
                  }
               }
               else{
                  data = NULL;
               }
            }
         }
      }
   }
   return data;
}
