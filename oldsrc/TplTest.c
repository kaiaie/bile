#include <libiberty.h>
#include <stdio.h>
#include <stdlib.h>
#include "Template.h"
#include "util.h"

int main(int argc, char *argv[]){
   FILE     *output   = NULL;
   Template *template = NULL;
   char     *programName = copyString(argv[0]);
   
   if(argc == 2 || argc == 3){
      fprintf(stderr, "Starting compile of %s... ", argv[1]);
      template = Template_compile(argv[1]);
      fprintf(stderr, "OK\n");
      Template_debugPrintTemplate(template);
      if(argc == 2){
         output = stdout;
      }
      else{
         if((output = fopen(argv[2], "w")) == NULL){
            perror("FATAL: main()");
            exit(EXIT_FAILURE);
         }
      }
      fprintf(stderr, "Executing template... ");
      Template_execute(template, output);
      fprintf(stderr, "OK\n");
      if(output != stdout) fclose(output);
      fprintf(stderr, "Cleaning up... ");
      delete_Template(template);
      fprintf(stderr, "OK\n");
   }
   else{
      fprintf(stderr, "Usage: %s template_file [output_file]\n\n", basename(programName));
      free(programName);
      exit(EXIT_FAILURE);
   }
   free(programName);
   exit(EXIT_SUCCESS);
}

