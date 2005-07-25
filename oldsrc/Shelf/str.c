#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "gc.h"
#include "str.h"

struct stringRec{
   String id;
   size_t size;
   char   *data;
};

static struct stringRec *stringPool = NULL;
static int  poolSize        = 0;
static int  poolUsed        = 0;
static bool poolInitialized = 0;

void shrinkPool(int level);
void deletePool(void);
int  sortPool(void *p1, void *p2);


static void initPool(){
   const int initialSize = 32;
   int i;
   struct stringRec *tmp = NULL;
   
   if(poolInitialized)return;

   /* Register garbage-collection functions */
   gcRegister(shrinkPool, deletePool);
   /* Set up pool */
   tmp = (struct stringRec *)gcMalloc((size_t)(initialSize * sizeof(struct stringRec)));
   for(i = 0; i < initialSize; ++i){
      tmp[i].id   = INVALID_STRING;
      tmp[i].size = 0;
      tmp[i].data = NULL;
   }
   stringPool      = tmp;
   poolSize        = initialSize;
   poolInitialized = true;
}


static void growPool(){
   size_t newSize;
   int    i;
   struct stringRec *tmp = NULL;
   
   newSize = poolSize * 2;
   
   tmp = (struct stringRec *)gcRealloc(stringPool, 
         (size_t)(newSize * sizeof(struct stringRec)));
   for(i = poolSize ; i < newSize; ++i){
      tmp[i].id   = INVALID_STRING;
      tmp[i].size = 0;
      tmp[i].data = NULL;
   }
   stringPool = tmp;
   poolSize   = newSize;
}


static String getNewID(){
   static String currID = INVALID_STRING;
   
   return ++currID;
}


static int getIndex(String s){
   int retVal = -1;
   int i;
   
   for(i = 0; i < poolSize; ++i){
      if(stringPool[i].id == s){
         retVal = i;
         break;
      }
   }
   return retVal;
}


static void growString(int idx, size_t newSize){
   char *tmp = NULL;
   
   if(idx < 0 || stringPool[idx].size > newSize) return;
   if(stringPool[idx].data == NULL){
      tmp = (char *)gcMalloc(newSize);
   }
   else{
      tmp = (char *)gcRealloc(stringPool[idx].data, newSize);
   }
   stringPool[idx].data = tmp;
   stringPool[idx].size = newSize;
}


void shrinkPool(int level){
   int    i;
   size_t strSize;
   char   *tmpChr = NULL;
   struct stringRec *tmp = NULL;
   
   switch(level){
      case 0:
         /* Free all unused strings */
         for(i = 0; i < poolSize; ++i){
            if(stringPool[i].id == INVALID_STRING && stringPool[i].data != NULL){
               free(stringPool[i].data);
               stringPool[i].size = 0;
               stringPool[i].data = NULL;
            }
         }
         break;
      case 1:
         /* Delete any unused space in the strings */
         for(i = 0; i < poolSize; ++i){
            if(stringPool[i].id != INVALID_STRING){
               strSize = strlen(stringPool[i].data + 1) * sizeof(char);
               if(stringPool[i].size > strSize){
                  tmpChr = (char *)realloc(stringPool[i].data, strSize);
                  if(tmp != NULL){
                     stringPool[i].data = tmpChr;
                     stringPool[i].size = strSize;
                  }
               }
            }
         }
         break;
      default:
         /* Compact the pool itself */
         if(poolSize > poolUsed){
            /* Sort all unused slots to the top... */
            qsort(stringPool, poolSize, sizeof(struct stringRec),
                  sortPool);
            /* ...and truncate */
            tmp = (struct stringRec *)realloc(stringPool, poolUsed);
            if(tmp != NULL){
               stringPool = tmp;
               poolSize   = poolUsed;
            }
         }
         break;
   }
}


int sortPool(void *p1, void *p2){
   int retVal = 0;
   String id1, id2;
   
   id1 = ((struct stringRec *)p1)->id;
   id2 = ((struct stringRec *)p2)->id;
   if(id1 == INVALID_STRING && id2 == INVALID_STRING){
      retVal = 0;
   }
   else if(id1 == INVALID_STRING){
      retVal = 1;
   }
   else if(id2 == INVALID_STRING){
      retVal = -1;
   }
   else{
      retVal = 0;
   }
   return retVal;
}


void deletePool(void){
   int i;
   for(i = 0; i < poolSize; ++i){
      if(stringPool[i].data != NULL){
         gcFree(stringPool[i].data);
         stringPool[i].data = NULL;
      }
   }
   gcFree(stringPool);
   poolInitialized = false;
}


bool compareString(String s1, String s2, int *cmp, bool caseBlind){
   bool retVal = false;
   char *c1 = NULL;
   char *c2 = NULL;
   
   if(String_getChars(s1, &c1) && String_getChars(s2, &c2)){
      if(caseBlind){
#ifdef __TURBOC__
         *cmp = strcmpi(c1, c2);
#else
         *cmp = strcasecmp(c1, c2);
#endif
      }
      else{
         *cmp   = strcmp(c1, c2);
      }
      retVal = true;
   }
   return retVal;
}


String new_String(const char *initial){
   String newString = INVALID_STRING;
   int    i;
   char   *tmp = NULL;
   
   if(!poolInitialized) initPool();
   if(poolUsed == (poolSize - 1)) growPool();
   newString = getNewID();
   if(newString != INVALID_STRING){
      for(i = 0; i < poolSize; ++i){
         if(stringPool[i].id == INVALID_STRING){
            break;
         }
      }
      stringPool[i].id = newString;
      if(stringPool[i].data != NULL){
         gcFree(stringPool[i].data);
         stringPool[i].size = 0;
         stringPool[i].data = NULL;
      }
      if(initial == NULL){
         stringPool[i].size = 0;
         stringPool[i].data = NULL;
      }
      else{
         stringPool[i].size = (strlen(initial) + 1) * sizeof(char);
         tmp = (char *)gcMalloc(stringPool[i].size);
         stringPool[i].data = tmp;
         strcpy(stringPool[i].data, initial);
      }
      poolUsed++;
   }
   return newString;
}


bool String_appendChar(String s, char ch){
   const  size_t initialSize = 8;
   bool   retVal = false;
   int    idx = -1;
   size_t strLen;
   size_t strSize;
   size_t newSize;
   
   idx = getIndex(s);
   if(idx >= 0){
      if(stringPool[idx].data == NULL){
         growString(idx, initialSize);
         strLen = 0;
      }
      else{
         strLen  = strlen(stringPool[idx].data);
      }
      strSize = strLen * sizeof(char);
      if(strSize == stringPool[idx].size){
         newSize = stringPool[idx].size * 2;
         growString(idx, newSize);
      }
      stringPool[idx].data[strLen] = ch;
      stringPool[idx].data[strLen + 1] = '\0';
   }
   return retVal;
}


bool String_appendString(String s1, String s2){
   bool   retVal = false;
   int    i1      = -1;
   char   *c1    = NULL;
   char   *c2    = NULL;
   size_t newSize;
   
   i1 = getIndex(s1);
   if(i1 >= 0){
      if(String_getChars(s1, &c1) && String_getChars(s2, &c2)){
         newSize = (strlen(c1) + strlen(c2) + 1) * sizeof(char);
         if(stringPool[i1].size < newSize){
            growString(i1, newSize);
         }
         strcat(stringPool[i1].data, c2);
         retVal = true;
      }
   }
   return retVal;
}


bool String_clear(String s1){
   bool retVal = false;
   char *tmp   = NULL;
   
   if(String_getChars(s1, &tmp)){
      if(tmp != NULL){
         tmp[0] = '\0';
      }
      retVal = true;
   }
   return retVal;
}


bool String_compare(String s1, String s2, int *cmp){
   return compareString(s1, s2, cmp, false);
}


bool String_compareIgnoreCase(String s1, String s2, int *cmp){
   return compareString(s1, s2, cmp, true);
}


bool String_copySubstring(String newStr, String oldStr, size_t startPos, size_t l){
   bool   retVal = false;
   char   *oldChr = NULL;
   char   *tmpChr = NULL;
   String tmpStr;
   
   if(String_getChars(oldStr, &oldChr)){
      tmpChr = (char *)gcMalloc((size_t)(l * sizeof(char) + 1));
      tmpChr[l] = '\0';
      strncpy(tmpChr, &oldChr[startPos], l);
      tmpStr = new_String(tmpChr);
      if(tmpStr != INVALID_STRING){
         retVal = String_copyString(newStr, tmpStr);
         delete_String(tmpStr);
      }
   }
   return retVal;
}


bool String_copyString(String newStr, String oldStr){
   bool   retVal  = false;
   char   *newChr = NULL;
   char   *oldChr = NULL;
   char   *tmpChr = NULL;
   size_t oldSize;  
   int    newIdx;
   
   if(String_getChars(newStr, &newChr) && String_getChars(oldStr, &oldChr)){
      if(oldChr != NULL){
         newIdx = getIndex(newStr);
         oldSize = (strlen(oldChr) + 1) * sizeof(char);
         if(newIdx >= 0){
            if(newStr == NULL){
               tmpChr = (char *)gcMalloc(oldSize);
               strcpy(tmpChr, oldChr);
               stringPool[newIdx].size = oldSize;
               stringPool[newIdx].data = tmpChr;
            }
            else{
               if(stringPool[newIdx].size < oldSize){
                  growString(newIdx, oldSize);
               }
               strcpy(stringPool[newIdx].data, oldChr);
               retVal = true;
            }
         }
      }
   }
   return retVal;
}


bool String_equals(String s1, String s2, bool *identical){
   bool retVal = false;
   int  cmp;
   
   retVal = compareString(s1, s2, &cmp, false);
   if(retVal){
      if(cmp == 0){
         *identical = true;
      }
      else{
         *identical = false;
      }
   }
   return retVal;
}


bool String_equalsIgnoreCase(String s1, String s2, bool *identical){
   bool retVal = false;
   int  cmp;
   
   retVal = compareString(s1, s2, &cmp, true);
   if(retVal){
      if(cmp == 0){
         *identical = true;
      }
      else{
         *identical = false;
      }
   }
   return retVal;
}


bool String_insertChar(String s, char ch, size_t atPos){
   bool   retVal = false;
   char   c[2];
   String tmp;
   
   c[0] = ch; c[1] = '\0';
   tmp  = new_String(c);
   if(tmp != INVALID_STRING){
      retVal = String_insertString(s, tmp, atPos);
      delete_String(tmp);
   }
   return retVal;
}


bool String_insertString(String s1, String s2, size_t atPos){
   bool   retVal = false;
   char   *c1    = NULL; size_t i1 = 0;
   char   *c2    = NULL; size_t i2 = 0;
   char   *c3    = NULL; size_t i3 = 0;
   int    state  = 0;
   size_t newSize;
   String tmp;
   
   if(String_getChars(s1, &c1) && String_getChars(s2, &c2)){
      newSize = (strlen(c1) + strlen(c2) + 1) * sizeof(char);
      c3      = (char *)gcMalloc(newSize);
      while(true){
         if(state == 0){
            if(i1 == atPos){
               state = 1;
            }
            else{
               c3[i3] = c1[i1];
               if(++i1 == strlen(c1)) break;
               i3++;
            }
         }
         else if(state == 1){
            if(i2 == strlen(c2)){
               state = 0;
            }
            else{
               c3[i3] = c2[i2];
               i2++; i3++;
            }
         }
      }
      c3[i3 + 1] = '\0';
      tmp = new_String(c3);
      if(tmp != INVALID_STRING){
         retVal = String_copyString(s1, tmp);
         delete_String(tmp);
      }
   }
   return retVal;
}


bool String_toUpper(String s){
   bool retVal = false;
   char *c1    = NULL;

   if(String_getChars(s, &c1)){
      while(*c1){
         *c1 = toupper(*c1);
         c1++;
      }
      retVal = true;
   }
   return retVal;
}


bool String_toLower(String s){
   bool retVal = false;
   char *c1    = NULL;

   if(String_getChars(s, &c1)){
      while(*c1){
         *c1 = tolower(*c1);
         c1++;
      }
      retVal = true;
   }
   return retVal;
}


bool String_trimAll(String s){
   return (String_trimLeft(s) && String_trimRight(s));
}


bool String_trimLeft(String s){
   bool   retVal = false;
   char   *c1    = NULL;
   size_t p1, p2;
   
   if(String_getChars(s, &c1)){
      p1 = 0; p2 = 0;
      while(isspace(c1[p2])) p2++;
      if(p2 != 0){
         while(c1[p2] != '\0'){
            c1[p1] = c1[p2];
            p1++; p2++;
         }
         c1[p1] = '\0';
         retVal = true;
      }
   }
   return retVal;
}


bool String_trimRight(String s){
   bool   retVal = false;
   char   *c1    = NULL;
   size_t theLength;
   size_t thePos;
   
   if(String_getChars(s, &c1)){
      theLength = strlen(c1);
      thePos = theLength;
      while(thePos >= 0 && isspace(c1[thePos])) thePos--;
      c1[thePos + 1] = '\0';
      retVal = true;
   }
   return retVal;
}


bool String_getCharAt(String s, size_t atPos, char *ch){
   bool retVal = false;
   char *c1    = NULL;
   
   if(String_getChars(s, &c1)){
      if((c1 != NULL) && (atPos < strlen(c1))){
         *ch = c1[atPos];
         retVal = true;
      }
   }
   return retVal;
}


bool String_getChars(String s, char **ch){
   bool retVal = false;
   int  idx;
   
   idx = getIndex(s);
   if(idx >= 0){
      *ch = stringPool[idx].data;
      retVal = true;
   }
   return retVal;
}


bool String_getIndexOf(String s, char ch, size_t startPos, size_t *atPos){
   bool retVal = false;
   char *c1    = NULL;
   size_t pos;
   
   if(String_getChars(s, &c1)){
      if((c1 != NULL) && (strlen(c1) > startPos)){
         pos = startPos;
         while(c1[pos]){
            if(c1[pos] == ch){
               *atPos = pos;
               retVal = true;
               break;
            }
            pos++;
         }
      }
   }
   return retVal;
}


bool String_getLength(String s, size_t *l){
   bool retVal = false;
   char *c1 = NULL;
   
   if(String_getChars(s, &c1)){
      *l     = strlen(c1);
      retVal = true;
   }
   return retVal;
}


void delete_String(String s){
   int idx;
   
   idx = getIndex(s);
   if(idx >= 0){
      stringPool[idx].id = INVALID_STRING;
      poolUsed--;
   }
}

