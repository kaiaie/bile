#include <stdlib.h>
#include "bool.h"
#include "gc.h"
#include "attrlist.h"
#include "str.h"

struct attrListRec{
   AttrList   id;
   size_t     size;
   size_t     length;
   String     *keys;
   int        *data;
};

static struct attrListRec *attrListPool = NULL;
static int  poolSize        = 0;
static int  poolUsed        = 0;
static bool poolInitialized = 0;

void shrinkPool(int level);
void deletePool(void);
int  sortPool(void *p1, void *p2);


static void initPool(){
   const int initialSize = 32;
   int i;
   struct attrListRec *tmp = NULL;
   
   if(poolInitialized)return;

   /* Register garbage-collection function */
   gcRegister(shrinkPool, deletePool);
   /* Set up pool */
   tmp = (struct attrListRec *)gcMalloc((size_t)(initialSize * sizeof(struct attrListRec)));
   for(i = 0; i < initialSize; ++i){
      tmp[i].id   = INVALID_ATTRLIST;
      tmp[i].size = 0;
      tmp[i].data = NULL;
   }
   attrListPool      = tmp;
   poolSize        = initialSize;
   poolInitialized = true;
}


static void growPool(){
   size_t newSize;
   int    i;
   struct attrListRec *tmp = NULL;
   
   newSize = poolSize * 2;
   
   tmp = (struct attrListRec *)gcRealloc(attrListPool, 
         (size_t)(newSize * sizeof(struct attrListRec)));
   for(i = poolSize ; i < newSize; ++i){
      tmp[i].id   = INVALID_ATTRLIST;
      tmp[i].size = 0;
      tmp[i].keys = NULL;
      tmp[i].data = NULL;
   }
   attrListPool = tmp;
   poolSize   = newSize;
}


static AttrList getNewID(){
   static AttrList currID = INVALID_ATTRLIST;
   
   return ++currID;
}


static int getIndex(AttrList l){
   int retVal = -1;
   int i;
   
   for(i = 0; i < poolSize; ++i){
      if(attrListPool[i].id == l){
         retVal = i;
         break;
      }
   }
   return retVal;
}


static void growAttrList(int idx, size_t newSize){
   int    *tmpData = NULL;
   String *tmpKeys = NULL;
   
   if(idx < 0 || attrListPool[idx].size > newSize) return;
   if(attrListPool[idx].data == NULL){
      tmpData = (int *)gcMalloc(newSize * sizeof(int));
   }
   else{
      tmpData = (int *)gcRealloc(attrListPool[idx].data, newSize * sizeof(int));
   }
   if(attrListPool[idx].keys == NULL){
      tmpKeys = (String *)gcMalloc(newSize * sizeof(String));
   }
   else{
      tmpKeys = (String *)gcRealloc(attrListPool[idx].keys, newSize * sizeof(String));
   }
   attrListPool[idx].data = tmpData;
   attrListPool[idx].keys = tmpKeys;
   attrListPool[idx].size = newSize;
}


void shrinkPool(int level){
   int    i;
   int   *tmpData = NULL;
   struct attrListRec *tmp = NULL;
   
   switch(level){
      case 0:
         /* Free all unused lists */
         for(i = 0; i < poolSize; ++i){
            if(attrListPool[i].id == INVALID_ATTRLIST){
               if(attrListPool[i].data != NULL){
                  free(attrListPool[i].data);
               }
               if(attrListPool[i].keys != NULL){
                  free(attrListPool[i].keys);
               }
               attrListPool[i].size = 0;
               attrListPool[i].keys = NULL;
               attrListPool[i].data = NULL;
            }
         }
         break;
      case 1:
         /* Delete any unused space in each list */
         for(i = 0; i < poolSize; ++i){
            if(attrListPool[i].id != INVALID_ATTRLIST){
               if(attrListPool[i].size > attrListPool[i].length){
                  tmpData = (int *)realloc(attrListPool[i].data, attrListPool[i].length * sizeof(int));
                  if(tmpData != NULL){
                     attrListPool[i].data = tmpData;
                     attrListPool[i].size = attrListPool[i].length;
                  }
               }
            }
         }
         break;
      default:
         /* Compact the pool itself */
         if(poolSize > poolUsed){
            /* Sort all unused slots to the top... */
            qsort(attrListPool, poolSize, sizeof(struct attrListRec),
                  sortPool);
            /* ...and truncate */
            tmp = (struct attrListRec *)realloc(attrListPool, poolUsed);
            if(tmp != NULL){
               attrListPool = tmp;
               poolSize   = poolUsed;
            }
         }
         break;
   }
}


int sortPool(void *p1, void *p2){
   int retVal = 0;
   AttrList id1, id2;
   
   id1 = ((struct attrListRec *)p1)->id;
   id2 = ((struct attrListRec *)p2)->id;
   if(id1 == INVALID_ATTRLIST && id2 == INVALID_ATTRLIST){
      retVal = 0;
   }
   else if(id1 == INVALID_ATTRLIST){
      retVal = 1;
   }
   else if(id2 == INVALID_ATTRLIST){
      retVal = -1;
   }
   else{
      retVal = 0;
   }
   return retVal;
}


void deletePool(void){
   int i;

   if(attrListPool != NULL){
      for(i = 0; i < poolSize; ++i){
         if(attrListPool[i].data != NULL){
            gcFree(attrListPool[i].data);
         }
         if(attrListPool[i].keys != NULL){
            gcFree(attrListPool[i].keys);
         }
      }
      gcFree(attrListPool);
   }
}


bool keyExists(AttrList l, String key){
   bool retVal = false;
   int  i;
   int  idx;
   
   idx = getIndex(l);
   if(idx >= 0){
      for(i = 0; i < attrListPool[idx].size; ++i){
         String_equalsIgnoreCase(key, attrListPool[idx].keys[i], &retVal);
         if(retVal){
            break;
         }
      }
   }
   return retVal;
}


AttrList new_AttrList(void){
   AttrList newList = INVALID_ATTRLIST;
   int      i;
   
   if(!poolInitialized) initPool();
   if(poolUsed == (poolSize - 1)) growPool();
   newList = getNewID();
   if(newList != INVALID_ATTRLIST){
      for(i = 0; i < poolSize; ++i){
         if(attrListPool[i].id == INVALID_ATTRLIST){
            break;
         }
      }
      attrListPool[i].id = newList;
      if(attrListPool[i].data != NULL){
         gcFree(attrListPool[i].data);
      }
      if(attrListPool[i].keys != NULL){
         gcFree(attrListPool[i].keys);
      }
      attrListPool[i].size = 0;
      attrListPool[i].keys = NULL;
      attrListPool[i].data = NULL;
      poolUsed++;
   }
   return newList;
}


bool AttrList_deleteItem(AttrList l, String key){
   bool   retVal = true;
   bool   found;
   bool   identical;
   String s;
   size_t i;
   int    idx = -1;

   idx = getIndex(l);
   if(idx >= 0){
      found     = false;
      for(i = 0; i < attrListPool[idx].size; ++i){
         s = attrListPool[idx].keys[i];
         if(s != INVALID_STRING){
            if(String_equalsIgnoreCase(s, key, &identical)){
               if(identical){
                  found = true;
                  break;
               }
            }
            else{
               break;
            }
         }
      }
      if(found){
         delete_String(attrListPool[idx].keys[i]);
         attrListPool[idx].keys[i] = INVALID_STRING;
         attrListPool[idx].length--;
         retVal = true;
      }
   }
   return retVal;
}


bool AttrList_replaceItem(AttrList l, String key, int v){
   bool retVal = true;
   
   if(keyExists(l, key)){
      retVal = AttrList_deleteItem(l, key);
   }
   if(retVal){
      retVal = AttrList_setItem(l, key, v);
   }
   return retVal;
}


bool AttrList_setItem(AttrList l, String key, int v){
   bool   retVal = false;
   int    idx    = -1;
   size_t i;
   String tmpKey;

   idx = getIndex(l);
   if(idx >= 0){
      if(!keyExists(l, key)){
         if(attrListPool[idx].length == (attrListPool[idx].size - 1)){
            growList(idx, (attrListPool[idx].size * 2));
         }
         for(i = 0; i < attrListPool[idx].size; ++i){
            if(attrListPool[idx].keys[i] == INVALID_STRING){
               break;
            }
         }
         tmpKey = new_String(NULL);
         if(tmpKey != INVALID_STRING && String_copyString(tmpKey, key)){
            attrListPool[idx].keys[i] = tmpKey;
            attrListPool[idx].data[i] = v;
            attrListPool[idx].length++;
            retVal = true;
         }
      }
   }
   return retVal;
}


bool AttrList_getItem(AttrList l, String key, int *v){
   bool   retVal = false;
   bool   found;
   bool   identical;
   int    idx = -1;
   int    i;
   
   idx = getIndex(l);
   if(idx >= 0){
      found = false;
      for(i = 0; i < attrListPool[idx].size; ++i){
         String_equalsIgnoreCase(key, attrListPool[idx].keys[i], &identical);
         if(identical){
            found = true;
            break;
         }
      }
      if(found){
         *v     = attrListPool[idx].data[i];
         retVal = true;
      }
   }
   return retVal;
}


bool AttrList_getLength(AttrList l, size_t *lgt){
   bool retVal = false;
   int  idx = -1;
   
   idx = getIndex(l);
   if(idx >= 0){
      *lgt   = attrListPool[idx].length;
      retVal = true;
   }
   return retVal;
}


void delete_AttrList(AttrList l){
   int idx;
   
   idx = getIndex(l);
   if(idx >= 0){
      attrListPool[idx].id = INVALID_ATTRLIST;
      poolUsed--;
   }
}

