#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bool.h"
#include "gc.h"
#include "str.h"
#include "var.h"
#ifdef __TURBOC__
#include "turboc.h"
#endif


struct varRec{
	Var           ID;
	Context       context;
	unsigned char flags;
	String        name;
	String        value;
};

static struct varRec *theHeap = NULL;
static bool heapInitialized = false;
static size_t heapSize = 0;
static size_t heapUsed = 0;

Context globalContext = 1;


void shrinkHeap(int level);
void deleteHeap(void);

void initHeap(void){
   const  size_t initialSize = 48;
   struct varRec *tmp = NULL;
   size_t i;
   String tn, tv;
   String fn, fv;
   String pn, pv;
   String en, ev;
   
   if(!heapInitialized){
      gcRegister(shrinkHeap, deleteHeap);
      tmp = (struct varRec *)gcMalloc(initialSize *
            sizeof(struct varRec));
      theHeap = tmp;
      for(i = 0; i < initialSize; i++){
         theHeap[i].ID      = 0;
         theHeap[i].context = 0;
         theHeap[i].name    = INVALID_STRING;
         theHeap[i].value   = INVALID_STRING;
      }
      heapSize = initialSize;
      heapUsed = 0;
      heapInitialized = true;
      /* Create values for True, False, and Pi, and the Error variable */
      tn = new_String("true");  tv = new_String("1");
      fn = new_String("false"); fv = new_String("0");
      pn = new_String("pi");    pv = new_String("3.14159265359");
      en = new_String("err");   ev = new_String("0");
      bileTrue  = Var_set(globalContext, tn, tv);
      bileFalse = Var_set(globalContext, fn, fv);
      bilePi    = Var_set(globalContext, pn, pv);
      bileErr   = Var_set(globalContext, en, ev);
   }
}


void deleteHeap(void){
   int i;
   
   if(heapInitialized){
      for(i = 0; i < heapSize; i++){
         if(theHeap[i].name != INVALID_STRING){
            delete_String(theHeap[i].name);
         }
         if(theHeap[i].value != INVALID_STRING){
            delete_String(theHeap[i].value);
         }
      }
      free(theHeap);
      theHeap = NULL;
      heapSize = 0;
      heapUsed = 0;
      heapInitialized = false;
   }
}


void shrinkHeap(int level){
   switch(level){
      default:
         if(heapSize != heapUsed){
         }
         break;
   }
}


Context getNewContext(void){
   static Context newContext = 1;
   
   return ++newContext;
}


Var getNewID(void){
   static Var newID = 0;
   
   return ++newID;
}


Var Var_set(Context context, String name, String value){
   size_t i;
   size_t newSize;
   Var newID;
   String n, v;
   bool createError;
   struct varRec *tmp = NULL;

   if(!heapInitialized) initHeap();

   createError = false;
   if((n = new_String(NULL)) == INVALID_STRING){
      createError = true;
   }
   else{
      if((v = new_String(NULL)) == INVALID_STRING){
         createError = true;
      }
      else{
         if(!String_copyString(n, name)){
            createError = true;
         }
         else{
            if(!String_copyString(v, value)){
               createError = true;
            }
         }
      }
   }
   if(createError){
      fprintf(stderr, "Var_set(): Cannot create new variable!\n");
      exit(EXIT_FAILURE);
   }
   if(heapSize == heapUsed){
      /* Resize heap */
      newSize = heapSize * 2;
      tmp = (struct varRec *)gcRealloc(theHeap, newSize *
            sizeof(struct varRec));
      theHeap = tmp;
      for(i = heapSize; i < newSize; i++){
         theHeap[i].ID      = 0;
         theHeap[i].context = 0;
         theHeap[i].name    = INVALID_STRING;
         theHeap[i].value   = INVALID_STRING;
      }
      heapSize = newSize;
   }

   for(i = 0; i < heapSize; i++){
      if(theHeap[i].ID == 0) break;
   }
   newID = getNewID();
   theHeap[i].ID      = newID;
   theHeap[i].context = context;
   theHeap[i].name    = n;
   theHeap[i].value   = v;
   heapUsed++;

   return newID;
}


Context new_Context(Context parentContext){
   Context newContext = INVALID_CONTEXT;
   String n, v;
   char buffer[16];

   newContext = getNewContext();
   if(newContext != INVALID_CONTEXT){
      n = new_String(".parent");
      if(n != INVALID_STRING){
         sprintf(buffer, "%-15d", parentContext);
         v = new_String(buffer);
         if(v != INVALID_STRING){
            String_trimRight(v);
            Var_set(newContext, n, v);
            delete_String(n);
            delete_String(v);
         }
         else{
            newContext = INVALID_CONTEXT;
         }
      }
      else{
         newContext = INVALID_CONTEXT;
      }
   }
   return newContext;
}


bool Context_getParent(Context context, Context *parent){
   int i;
   String n    = INVALID_STRING;
   char *v     = NULL;
   int cmp     = 0;
   bool retVal = false;

   n = new_String(".parent");
   if(n != INVALID_STRING){
      if(context == globalContext){
         *parent = INVALID_CONTEXT;
         retVal = true;
      }
      else{
         for(i = 0; i < heapSize; i++){
            if(theHeap[i].context == context){
               if(String_compare(theHeap[i].name, n, &cmp)){
                  if(cmp == 0){
                     if(String_getChars(theHeap[i].value, &v)){
                        *parent = (Context)atoi(v);
                        retVal = true;
                        break;
                     }
                  }
               }
            }
         }
      }
      delete_String(n);
   }
   return retVal;
}


int getOrdinal(Var ID){
   int i;
   int retVal = -1;

   for(i = 0; i < heapSize; i++){
      if(theHeap[i].ID == ID){
         retVal = i;
         break;
      }
   }
   return retVal;
}


void deleteVariableByOrdinal(int ord){
	if(ord >= 0 && ord < heapSize){
		theHeap[ord].ID = 0;
		theHeap[ord].context = 0;
		if(theHeap[ord].name != INVALID_STRING){
		   delete_String(theHeap[ord].name);
		}
		if(theHeap[ord].value != INVALID_STRING){
			delete_String(theHeap[ord].value);
		}
		heapUsed--;
	}
}


void delete_Var(Var v){
	deleteVariableByOrdinal(getOrdinal(v));
}


void delete_Context(Context context){
	int i;

	for(i = 0; i < heapSize; i++){
		if(theHeap[i].context == context){
			deleteVariableByOrdinal(i);
		}
	}
}


Var findVar(Context originalContext, Context currContext, String name){
   int    i;
   int    eq;
   String tmp;
   char   *ch = NULL;

   for(i = 0; i < heapSize; i++){
      if(theHeap[i].context == currContext){
         eq = 1;
         String_compare(theHeap[i].name, name, &eq);
         if(eq == 0){
            return theHeap[i].ID;
         }
      }
   }
   if(currContext != globalContext){
      /* Nothing doing... try the parent context              */
      return findVar(originalContext, getParentContext(currContext),
            name);
   }
   else{
      /* If it's not in the global context, the only other    */
      /* place we can try is the environment.  If an          */
      /* environment variable of the requested name exists,   */
      /* make a copy in the original context and return the   */
      /* ID of the copy.                                      */
      String_getChars(name, &ch);
      tmp = new_String(getenv(ch));
      return Var_set(originalContext, name, tmp);
   }
}


Var Var_find(Context context, String name){
   return findVar(context, context, name);
}


bool Var_getStringValue(Var v, String *value){
   int ord;
   bool retVal = false;
   
   ord = getOrdinal(v);
   if(ord >= 0){
      *value = theHeap[ord].value;
      retVal = true;
   }
   return retVal;
}


bool Var_getIntValue(Var v, int *value){
   int ord;
   char *ch = NULL;
   bool retVal = false;

   ord = getOrdinal(v);
   if(ord >= 0){
      if(theHeap[ord].value == INVALID_STRING){
         *value = 0;
         retVal = true;
      }
      else{
         if(String_getChars(theHeap[ord].value, &ch)){
            if(ch == NULL){
               *value = 0;
               retVal = true;
            }
            else{
               *value = atoi(ch);
               retVal = true;
            }
         }
         else{
            retVal = false;
         }
      }
   }
   return retVal;
}


bool Var_getDoubleValue(Var v, double *value){
   int ord;
   char *ch = NULL;
   bool retVal = false;

   ord = getOrdinal(v);
   if(ord >= 0){
      if(theHeap[ord].value == INVALID_STRING){
         *value = 0.0;
         retVal = true;
      }
      else{
         if(String_getChars(theHeap[ord].value, &ch)){
            if(ch == NULL){
               *value = 0.0;
               retVal = true;
            }
            else{
               *value = atof(ch);
               retVal = true;
            }
         }
         else{
            retVal = false;
         }
      }
   }
   return retVal;
}


bool Var_getBoolValue(Var v, bool *value){
   int ord;
   int tmp = 0;
   bool retVal = false;

   ord = getOrdinal(v);
   if(ord >= 0){
      if(theHeap[ord].value == INVALID_STRING){
         *value = false;
         retVal = true;
      }
      else{
         if(Var_getIntValue(v, &tmp)){
            if(tmp == 0){
               *value = false;
            }
            else{
               *value = true;
            }
            retVal = true;
         }
         else{
            retVal = false;
         }
      }
   }
   return retVal;
}


bool Var_getDateValue(Var v, time_t *value){
   int ord;
   int i;
   bool isValid = false;
   bool retVal = false;
   struct tm theDate;
   char *ch = NULL;
   char yearPart[5];
   char monthPart[3];
   char dayPart[3];
   char hourPart[3];
   char minPart[3];
   char secPart[3];

   ord = getOrdinal(v);
   if(ord >= 0 && theHeap[ord].value != NULL){
      /* Check if string is valid ISO8601 local date          */
      /* (yyyymmddThhmmss)                                    */
      if(String_getChars(theHeap[ord].value, &ch)){
         if(strlen(ch) == 15){
            isValid = true;
            for(i = 0; i < 15; i++){
               if(i == 8){
                  if(ch[i] != 'T'){
                     isValid = false;
                     break;
                  }
               }
               else{
                  if(!isdigit(ch[i])){
                     isValid = false;
                     break;
                  }
               }
            }
            if(isValid){
               /* Extract parts of date                          */
               strncpy(yearPart,  &ch[0],  4); yearPart[4]  = '\0';
               strncpy(monthPart, &ch[4],  2); monthPart[2] = '\0';
               strncpy(dayPart,   &ch[6],  2); dayPart[2]   = '\0';
               strncpy(hourPart,  &ch[9],  2); hourPart[2]  = '\0';
               strncpy(minPart,   &ch[11], 2); minPart[2]   = '\0';
               strncpy(secPart,   &ch[13], 2); secPart[2]   = '\0';
               /* Populate tm structure */
               theDate.tm_year = atoi(yearPart) - 1900;
               theDate.tm_mon  = atoi(monthPart) - 1;
               theDate.tm_mday = atoi(dayPart);
               theDate.tm_hour = atoi(hourPart);
               theDate.tm_min  = atoi(minPart);
               theDate.tm_sec  = atoi(secPart);
               /* Convert into time_t */
      			*value = mktime(&theDate);
      			retVal = true;
            }
         }
      }
   }
   return retVal;
}


bool Var_setFlags(Var v, unsigned char flags){
   bool retVal = true;
   int  ord;
   
   ord = getOrdinal(v);
   if(ord >= 0){
      theHeap[ord].flags = flags;
   }
   else{
      retVal = false;
   }
   
   return retVal;
}


bool Var_getFlags(Var v, unsigned char *flags){
   bool retVal = true;
   int  ord;
   
   ord = getOrdinal(v);
   if(ord >= 0){
      *flags = theHeap[ord].flags;
   }
   else{
      retVal = false;
   }
   
   return retVal;
}


void Var_dumpHeap(){
   size_t i;

   printf("Heap contents\n");
   printf("%-4s %-4s %-4s %-40s %s\n", "Ord", "ID", "Ctxt", "Name", "Value");
   for(i = 0; i < heapSize; i++){
      if(theHeap[i].ID == 0){
         printf("%4d (Empty)\n", i);
      }
      else{
		 printf("%4d %4d %4d %-40s %s\n", i,
               theHeap[i].ID,
               theHeap[i].context,
               theHeap[i].name,
               theHeap[i].value);
      }
   }
}
