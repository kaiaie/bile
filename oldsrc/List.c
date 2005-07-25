#include <stdio.h>
#include <stdlib.h>
#include "List.h"
#include "util.h"

ListNode *newListNode(void *data);
void     deleteListNode(ListNode *ln, bool freeData);
ListNode *listGetNode(List *l, size_t index, ListNode **prevNode);
size_t   listNormalizeIndex(List *l, int index);


List *new_List(void){
   static const char fnName[] __attribute__((unused)) = "new_List()";
   List *retVal = NULL;
   
   if((retVal = (List *)malloc(sizeof(List))) == NULL){
      fatal(fnName, "Out of memory!");
   }
   retVal->length = 0;
   retVal->first  = NULL;
   return retVal;
}


void delete_List(List *l, bool freeData){
   static const char fnName[] __attribute__((unused)) = "delete_List()";

   if(l != NULL){
      while(l->length > 0){
         List_remove(l, 0, freeData);
      }
      free(l);
   }
}


size_t List_length(List *l){
   static const char fnName[] __attribute__((unused)) = "List_length()";
   size_t retVal = 0;
   
   if(l != NULL){
      retVal = l->length;
   }
   return retVal;
}


void List_insert(List *l, int index, void *data){
   static const char fnName[] __attribute__((unused)) = "List_insert()";
   size_t   idx = 0;
   ListNode *nn = NULL;
   ListNode *cc = NULL;
   ListNode *pp = NULL;
   
   if(l != NULL){
      idx = listNormalizeIndex(l, index);
      nn = newListNode(data);
      if(idx == 0){
         if(l->length != 0){
            nn->next = l->first;
         }
         l->first = nn;
      }
      else{
         cc = listGetNode(l, idx, &pp);
         pp->next = nn;
         nn->next = cc;
      }
      l->length++;
   }
}


void List_append(List *l, void *data){
   List_insert(l, -1, data);
}


void List_remove(List *l, int index, bool freeData){
   static const char fnName[] __attribute__((unused)) = "List_remove()";
   size_t   idx = 0;
   ListNode *cc = NULL;
   ListNode *pp = NULL;
   
   if(l != NULL){
      idx = listNormalizeIndex(l, index);
      if(idx == 0){
         cc = l->first;
         l->first = cc->next;
      }
      else{
         cc = listGetNode(l, idx, &pp);
         pp->next = cc->next;
      }
      deleteListNode(cc, freeData);
      l->length--;
   }
}


ListNode *newListNode(void *data){
   static const char fnName[] __attribute__((unused)) = "newListNode()";
   ListNode *retVal = NULL;
   
   if((retVal = (ListNode *)malloc(sizeof(ListNode))) == NULL){
      fatal(fnName, "Out of memory!");
   }
   retVal->data = data;
   retVal->next = NULL;
   
   return retVal;
}


void deleteListNode(ListNode *ln, bool freeData){
   if(ln != NULL){
      if(freeData && ln->data != NULL) free(ln->data);
      free(ln);
   }
}


ListNode *listGetNode(List *l, size_t index, ListNode **prevNode){
   static const char fnName[] __attribute__((unused)) = "listGetNode()";
   ListNode *thisNode = NULL;
   ListNode *lastNode = NULL;
   size_t   ii        = 0;
   
   if(l != NULL){
      thisNode = l->first;
      while(ii < index){
         lastNode = thisNode;
         thisNode = thisNode->next;
         ii++;
      }
   }
   if(prevNode != NULL) *prevNode = lastNode;
   return thisNode;
}


size_t listNormalizeIndex(List *l, int index){
   static const char fnName[] __attribute__((unused)) = "listNormalizeIndex()";
   size_t retVal = 0;

   if(l != NULL){
      if(index >= 0){
         retVal = index;
      }
      else{
         retVal = l->length - (abs(index) - 1);
      }
      if(retVal > l->length){
         fatal(fnName, "Out of range.");
      }
   }
   else{
      fatal(fnName, "Null list.");
   }
   return retVal;
}
