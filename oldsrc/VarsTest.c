#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "Vars.h"

int main(int argc, char *argv[]){
   Vars *v1 = NULL;
   
   v1 = new_Vars();
   if(v1 == NULL){
      fatal("main", "v1 not initialized.");
   }
   else{
      fprintf(stderr, "v1 initialized.\n");
   }
   Vars_putString(v1, "foo", "bar");
   Vars_putString(v1, "fred", "123");
   Vars_dump(v1);
   exit(EXIT_SUCCESS);
}
