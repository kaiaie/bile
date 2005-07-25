/* :tabSize=3:indentSize=3: */
/* astring.c -- String functions that allocate strings; it's the     */
/*              caller's responsibility to free() them.              */
#include <stdlib.h>
#include <string.h>
#include "util.h"

char *astrcpy(const char *src){
	/* Returns a new copy of the string passed to it */
	static const char fnName[] __attribute__((unused)) = "astrcpy()";
	char *inputCopy = NULL;
	
	if((inputCopy = (char *)malloc((strlen(src) + 1) * sizeof(char))) == NULL){
		fatal(fnName, "Out of memory!");
	}
	else{
		strcpy(inputCopy, src);
	}
	return inputCopy;
}


char *astrcat(const char *src1, const char *src2){
	/* Returns a new string comprising of the two strings passed */
	/* to it joined together.                                    */
	static const char fnName[] __attribute__((unused)) = "astrcat()";
	char   *catString = NULL;
	size_t catLength  = strlen(src1) + strlen(src2) + 1;
	
	if((catString = (char *)malloc(catLength * sizeof(char))) == NULL){
		fatal(fnName, "Out of memory!");
	}
	else{
		strcpy(catString, src1);
		strcat(catString, src2);
	}
	return catString;
}


char *astrleft(const char *src, size_t count){
	/* Returns a new string consisting of the leftmost count          */
	/* characters of the source string.                               */
	static const char fnName[] __attribute__((unused)) = "astrleft()";
	char   *leftString = NULL;
	size_t actualCount;
	
	if(count == 0){
		return NULL;
	}
	else if(count >= strlen(src)){
		return astrcpy(src);
	}
	else{
		if((leftString = (char *)malloc((count + 1) * sizeof(char))) == NULL){
			fatal(fnName, "Out of memory!");
		}
		else{
			if(count >= strlen(src)){
				actualCount = strlen(src);
			}
			else{
				actualCount = count;
			}
			memcpy(leftString, src, actualCount);
			leftString[actualCount] = '\0';
		}
	}
	return leftString;
}


char *astrmid(const char *src, size_t offset, size_t count){
	/* Returns a new string consisting of the count characters        */
	/* starting at offset in the source string.                       */
	/* If count is zero, all characters to the end of the string are  */
	/* returned.                                                      */
	static const char fnName[] __attribute__((unused)) = "astrmid()";
	char *midString = NULL;
	
	size_t actualCount;
	if(offset >= strlen(src)){
		return NULL;
	}
	else{
		if((count == 0) || (offset + count > strlen(src))){
			actualCount = strlen(src) - offset;
		}
		else{
			actualCount = count;
		}
		if((midString = (char *)malloc((actualCount + 1) * sizeof(char))) == NULL){
			fatal(fnName, "Out of memory!");
		}
		else{
			memcpy(midString, &src[offset], actualCount);
			midString[actualCount] = '\0';
		}
	}
	return midString;
}


char *astrright(const char *src, size_t count){
	/* Returns a new string consisting of the rightmost count         */
	/* characters of the source string.                               */
	static const char fnName[] __attribute__((unused)) = "astrright()";
	char   *rightString = NULL;
	
	if(count >= strlen(src)){
		return astrcpy(src);
	}
	else{
		return astrcpy(&src[strlen(src) - count]);
	}
	
	return rightString;
}


int main(int argc, char *argv[]){
	char testString[] = "This is a test";
	char *aCopy    = NULL;
	char *leftBit  = NULL;
	char *midBit   = NULL;
	char *rightBit = NULL;
	
	aCopy = astrcpy(testString);
	if(aCopy != NULL){
		leftBit = astrleft(testString, 4);
		if(leftBit != NULL){
			printf("Left(4): \"%s\"\n", leftBit);
		}
		else{
			printf("Error calling astrleft()\n");
		}
		midBit = astrmid(aCopy, 2, 6);
		if(midBit != NULL){
			printf("Mid(2, 6): \"%s\"\n", midBit);
		}
		else{
			printf("Error calling astrmid()\n");
		}
		rightBit = astrright(aCopy, 2);
		if(rightBit != NULL){
			printf("Right(2): \"%s\"\n", rightBit);
		}
		else{
			printf("Error calling astrright()\n");
		}
	}
	if(aCopy != NULL) free(aCopy);
	if(leftBit != NULL) free(leftBit);
	if(midBit != NULL) free(midBit);
	if(rightBit != NULL) free(rightBit);
	exit(EXIT_SUCCESS);
}
