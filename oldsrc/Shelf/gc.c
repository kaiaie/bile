#include <stdio.h>
#include <stdlib.h>
#include "bool.h"
#include "gc.h"
#include "global.h"

struct gcRec{
   void (*compactor)(int level);
   void (*destructor)(void);
   struct gcRec *next;
};

static struct gcRec *cptList = NULL;
static bool   gcInitialized = false;

void gcDestroy(void);

static void gcInit(void){
   if(!gcInitialized){
      atexit(gcDestroy);
      gcInitialized = true;
   }
}


bool gcRegister(void (*compactor)(int level), void(*destructor)(void)){
   bool retVal = true;
   struct gcRec *p    = cptList;
   struct gcRec *last = NULL;
   struct gcRec *tmp  = NULL;
   
   if(!gcInitialized)gcInit();
   while(p != NULL){
      last = p;
      p    = p->next;
   }
   tmp = (struct gcRec *)malloc(sizeof(struct gcRec));
   if(tmp == NULL){
      fprintf(stderr, "FATAL: gcRegister(): Out of memory!\n");
      exit(EXIT_FAILURE);
   }
   tmp->compactor = compactor;
   tmp->destructor = destructor;
   tmp->next = NULL;
   if(cptList == NULL){
      cptList = tmp;
   }
   else{
      last->next = tmp;
   }
   return retVal;
}


void gcCompact(int level){
   struct gcRec *p = cptList;
   
   if(!gcInitialized)gcInit();
   while(p != NULL){
      if(p->compactor != NULL){
         (p->compactor)(level);
      }
      p = p->next;
   }
}


void gcFreeList(struct gcRec *p){
   if(p->next != NULL){
      gcFreeList(p->next);
      p->next = NULL;
   }
   free(p);
}


void gcDestroy(void){
   struct gcRec *p    = cptList;
   
   if(!gcInitialized)gcInit();
   if(debugMode){
      fprintf(stderr, "gcDestroy() called\n");
   }
   while(p != NULL){
      if(p->destructor != NULL){
         (p->destructor)();
      }
      p = p->next;
   }
   if(cptList != NULL){
      if(cptList->next != NULL){
         gcFreeList(cptList->next);
      }
      free(cptList);
      cptList = NULL;
   }
   gcInitialized = false;
}


void *gcMalloc(size_t size){
   int  i;
   void *p = NULL;
   
   if(!gcInitialized)gcInit();
   for(i = 0; i < 4; ++i){
      p = malloc(size);
      if(p == NULL){
         gcCompact(i);
      }
      else{
         break;
      }
   }
   if(p == NULL){
      fprintf(stderr, "FATAL: gcMalloc(): Out of memory!\n");
      exit(EXIT_FAILURE);
   }
   return p;
}


void *gcRealloc(void *block, size_t size){
   int  i;
   void *p = NULL;
   
   if(!gcInitialized)gcInit();
   for(i = 0; i < 4; ++i){
      p = realloc(block, size);
      if(p != NULL){
         gcCompact(i);
         break;
      }
   }
   if(p == NULL){
      fprintf(stderr, "FATAL: gcRealloc(): Out of memory!\n");
      exit(EXIT_FAILURE);
   }
   return p;
}


void gcFree(void *block){
   if(!gcInitialized)gcInit();
   free(block);
}

