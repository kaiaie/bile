#include <stdio.h>
#include <stdlib.h>
#include "Env.h"

int main(int argc, char *argv[]){
   Env *e1 = new_Env(NULL);
   Env *e2 = new_Env(e1);
   
   Env_setString(e1, "$abc", "def");
   Env_setString(e1, "$def", "123");
   Env_setString(e2, "$abc", "different");
   
   printf("abc: %s\n", Env_getString(e2,"$abc"));
   printf("path: %s\n", Env_getString(e2,"$PATH"));
   printf("unknown: %s\n", Env_getString(e2,"$unknown"));
   printf("def: %s\n", Env_getString(e2,"$def"));

   exit(EXIT_SUCCESS);
}
