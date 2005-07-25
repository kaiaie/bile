#include <stdlib.h>
#include "bool.h"
#include "gc.h"
#include "list.h"

struct listRec{
   List   id;
   size_t size;
   size_t length;
   int   *data;
};

static struct listRec *listPool = NULL;
static int  poolSize        = 0;
static int  poolUsed        = 0;
static bool poolInitialized = 0;

void shrinkPool(int level);
void deletePool(void);
int  sortPool(void *p1, void *p2);


static void initPool(){
   const int initialSize = 32;
   int i;
   struct listRec *tmp = NULL;
   
   if(poolInitialized)return;

   /* Register garbage-collection function */
   gcRegister(shrinkPool, deletePool);
   /* Set up pool */
   tmp = (struct listRec *)gcMalloc((size_t)(initialSize * sizeof(struct listRec)));
   for(i = 0; i < initialSize; ++i){
      tmp[i].id   = INVALID_LIST;
      tmp[i].size = 0;
      tmp[i].data = NULL;
   }
   listPool      = tmp;
   poolSize        = initialSize;
   poolInitialized = true;
}


static void growPool(){
   size_t newSize;
   int    i;
   struct listRec *tmp = NULL;
   
   newSize = poolSize * 2;
   
   tmp = (struct listRec *)gcRealloc(listPool, 
         (size_t)(newSize * sizeof(struct listRec)));
   for(i = poolSize ; i < newSize; ++i){
      tmp[i].id   = INVALID_LIST;
      tmp[i].size = 0;
      tmp[i].data = NULL;
   }
   listPool = tmp;
   poolSize   = newSize;
}


static List getNewID(){
   static List currID = INVALID_LIST;
   
   return ++currID;
}


static int getIndex(List l){
   int retVal = -1;
   int i;
   
   for(i = 0; i < poolSize; ++i){
      if(listPool[i].id == l){
         retVal = i;
         break;
      }
   }
   return retVal;
}


static void growList(int idx, size_t newSize){
   int *tmp = NULL;
   
   if(idx < 0 || listPool[idx].size > newSize) return;
   if(listPool[idx].data == NULL){
      tmp = (int *)gcMalloc(newSize * sizeof(int));
   }
   else{
      tmp = (int *)gcRealloc(listPool[idx].data, newSize * sizeof(int));
   }
   listPool[idx].data = tmp;
   listPool[idx].size = newSize;
}


void shrinkPool(int level){
   int    i;
   int   *tmpData = NULL;
   struct listRec *tmp = NULL;
   
   switch(level){
      case 0:
         /* Free all unused lists */
         for(i = 0; i < poolSize; ++i){
            if(listPool[i].id == INVALID_LIST && listPool[i].data != NULL){
               free(listPool[i].data);
               listPool[i].size = 0;
               listPool[i].data = NULL;
            }
         }
         break;
      case 1:
         /* Delete any unused space in each list */
         for(i = 0; i < poolSize; ++i){
            if(listPool[i].id != INVALID_LIST){
               if(listPool[i].size > listPool[i].length){
                  tmpData = (int *)realloc(listPool[i].data, listPool[i].length * sizeof(int));
                  if(tmp != NULL){
                     listPool[i].data = tmpData;
                     listPool[i].size = listPool[i].length;
                  }
               }
            }
         }
         break;
      default:
         /* Compact the pool itself */
         if(poolSize > poolUsed){
            /* Sort all unused slots to the top... */
            qsort(listPool, poolSize, sizeof(struct listRec),
                  sortPool);
            /* ...and truncate */
            tmp = (struct listRec *)realloc(listPool, poolUsed);
            if(tmp != NULL){
               listPool = tmp;
               poolSize   = poolUsed;
            }
         }
         break;
   }
}


int sortPool(void *p1, void *p2){
   int retVal = 0;
   List id1, id2;
   
   id1 = ((struct listRec *)p1)->id;
   id2 = ((struct listRec *)p2)->id;
   if(id1 == INVALID_LIST && id2 == INVALID_LIST){
      retVal = 0;
   }
   else if(id1 == INVALID_LIST){
      retVal = 1;
   }
   else if(id2 == INVALID_LIST){
      retVal = -1;
   }
   else{
      retVal = 0;
   }
   return retVal;
}


void deletePool(void){
   int i;

   if(listPool != NULL){
      for(i = 0; i < poolSize; ++i){
         if(listPool[i].data != NULL){
            gcFree(listPool[i].data);
         }
      }
      gcFree(listPool);
   }
}


size_t getActualPos(List l, int pos){
   size_t retVal = 0;
   size_t listLen;
   
   if(pos >= 0){
      retVal = (size_t)pos;
   }
   else{
      if(List_getLength(l, &listLen)){
         retVal = listLen + (size_t)pos + 1;
      }
   }
   return retVal;
}


List new_List(void){
   List newList = INVALID_LIST;
   int    i;
   
   if(!poolInitialized) initPool();
   if(poolUsed == (poolSize - 1)) growPool();
   newList = getNewID();
   if(newList != INVALID_LIST){
      for(i = 0; i < poolSize; ++i){
         if(listPool[i].id == INVALID_LIST){
            break;
         }
      }
      listPool[i].id = newList;
      if(listPool[i].data != NULL){
         gcFree(listPool[i].data);
         listPool[i].size = 0;
         listPool[i].data = NULL;
      }
      poolUsed++;
   }
   return newList;
}


bool List_appendItem(List l, int v){
   return List_insertItem(l, -1, v);
}


bool List_deleteItem(List l, int pos){
   bool   retVal = true;
   size_t actualPos;
   size_t i, j;
   int    idx = -1;

   idx = getIndex(l);
   if(idx >= 0){
      actualPos = getActualPos(l, pos);
      j = 0;
      for(i = 0; i < listPool[idx].length; ++i){
         if(i != actualPos){
            listPool[idx].data[j] = listPool[idx].data[i];
            j++;
         }
      }
      listPool[idx].length--;
      retVal = true;
   }
   
   return retVal;
}


bool List_insertItem(List l, int pos, int v){
   bool   retVal = false;
   int    idx;
   size_t actualPos;
   size_t i, j;
   int    *tmp = NULL;

   idx = getIndex(l);
   if(idx >= 0){
      actualPos = getActualPos(l, pos);
      tmp = (int *)gcMalloc(listPool[idx].length * sizeof(int));
      for(i = 0; i < listPool[idx].length; ++i){
         tmp[i] = listPool[idx].data[i];
      }
      if(listPool[idx].length == (listPool[idx].size - 1)){
         growList(idx, listPool[idx].length * 2);
      }
      j = 0;
      for(i = 0; i <= listPool[idx].length; ++i){
         if(i == actualPos){
            listPool[idx].data[i] = v;
         }
         else{
            listPool[idx].data[i] = tmp[j];
            j++;
         }
      }
      listPool[idx].length++;
      gcFree(tmp);
   }
   return retVal;
}


bool List_setItem(List l, int pos, int v){
   bool   retVal = false;
   size_t actualPos;
   int    idx = -1;
   
   idx = getIndex(l);
   if(idx >= 0){
      actualPos = getActualPos(l, pos);
      if(actualPos < listPool[idx].length){
         listPool[idx].data[actualPos] = v;
         retVal = true;
      }
   }
   return retVal;
}


bool List_getItem(List l, int pos, int *v){
   bool   retVal = false;
   size_t actualPos;
   int    idx = -1;
   
   idx = getIndex(l);
   if(idx >= 0){
      actualPos = getActualPos(l, pos);
      if(actualPos < listPool[idx].length){
         *v = listPool[idx].data[actualPos];
         retVal = true;
      }
   }
   return retVal;
}


bool List_getLength(List l, size_t *lgt){
   bool retVal = false;
   int  idx = -1;
   
   idx = getIndex(l);
   if(idx >= 0){
      *lgt   = listPool[idx].length;
      retVal = true;
   }
   return retVal;
}


void delete_List(List l){
   int idx;
   
   idx = getIndex(l);
   if(idx >= 0){
      listPool[idx].id = INVALID_LIST;
      poolUsed--;
   }
}

