#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "Buffer.h"
#include "util.h"

void visitDir(const char *dirName, Buffer *b){
   DIR           *d  = NULL;
   struct dirent *de = NULL;
   struct stat   fileInfo;
   char          *subDir = NULL;
   
   d = opendir(dirName);
   if(d != NULL){
      while((de = readdir(d)) != NULL){
         if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){
            Buffer_reset(b);
            Buffer_appendString(b, dirName);
            Buffer_appendChar(b, '\\');
            Buffer_appendString(b, de->d_name);
            printf("%s\n", b->data);
            stat(b->data, &fileInfo);
            if(S_ISDIR(fileInfo.st_mode)){
               subDir = copyString(b->data);
               visitDir(subDir, b);
               free(subDir);
            }
         }
      }
      closedir(d);
   }
}

int main(int argc, char *argv[]){
   const char startDir[] = "E:\\Docs\\Ken\\Projects\\C\\BILE\\Test\\Input";
   Buffer *b = new_Buffer(MAX_PATH);
   visitDir(startDir, b);
	exit(EXIT_SUCCESS);
}
