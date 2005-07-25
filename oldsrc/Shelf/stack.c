#include "bool.h"
#include "list.h"
#include "stack.h"

List new_Stack(void){
	return (Stack)new_List();
}


void delete_Stack(Stack s){
   delete_List((List)s);
}


bool Stack_dup(Stack s){
	bool retVal = true;
	int  tmp;

   if(List_getItem((List)s, -1, &tmp)){
      if(List_setItem((List)s, -1, tmp)){
         retVal = true;
      }
   }
	return retVal;
}


bool Stack_pop(Stack s, int *v){
	bool retVal = true;
	int  tmp;
	
	if(List_getItem((List)s, -1, &tmp)){
	   if(List_deleteItem((List)s, -1)){
	      *v = tmp;
	      retVal = true;
	   }
	}
	return retVal;
}


bool Stack_push(Stack s, int v){
	return List_appendItem((List)s, v);
}


bool Stack_getLength(Stack s, size_t *lgt){
	return List_getLength((List)s, lgt);
}

