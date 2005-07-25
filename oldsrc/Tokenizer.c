/* :tabSize=3:indentSize=3: */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "Buffer.h"
#include "Tokenizer.h"
#include "util.h"
#define INITIAL_SIZE 24

/* -------------------------------------------------------------------
 * Local function declarations
 * ------------------------------------------------------------------- */

void eatWhitespace(Tokenizer *t);
void getSymbol(Tokenizer *t, Buffer *b);
void getNumber(Tokenizer *t, Buffer *b);
void getVariable(Tokenizer *t, Buffer *b);
void getString(Tokenizer *t, Buffer *b);
void getOperator(Tokenizer *t, Buffer *b);
void initialize(void);


/* -------------------------------------------------------------------
 * Local variables
 * ------------------------------------------------------------------- */

static bool       initialized   = false;
static const char symbolChars[] = "+-*^/.";


/* -------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------- */

Tokenizer *new_Tokenizer(char *input){
   static const char fnName[] __attribute__((unused)) = "new_Tokenizer()";
   Tokenizer *t         = NULL;
   char      *inputCopy = NULL;

   if(!initialized) initialize();
   if((t = (Tokenizer *)malloc(sizeof(Tokenizer))) == NULL){
      fprintf(stderr, "FATAL: %s: Out of memory!\n", fnName);
      exit(EXIT_FAILURE);
   }
   t->input         = NULL;
   t->currPos       = 0;
   t->hasMoreTokens = false;
   t->currToken     = NULL;
   if(input != NULL && strlen(input) != 0 && !isAllWhitespace(input)){
      /* Copy input string, skipping any initial whitespace */
      while(isWhitespace(input[0])) input++;
      if((inputCopy = copyString(input)) == NULL){
         fprintf(stderr, 
               "FATAL: %s: Cannot allocate space for input string.\n",
               fnName);
         exit(EXIT_FAILURE);
      }
      t->input         = inputCopy;
      t->hasMoreTokens = true;
   }
   else{
      fprintf(stderr, 
            "DEBUG: %s: Tokenizer created with empty string.\n", 
            fnName);
      t->hasMoreTokens = false;
   }
   return t;
} /* new_Tokenizer */


void delete_Tokenizer(Tokenizer *t){
   static const char fnName[] __attribute__((unused)) = "delete_Tokenizer()";
   if(!initialized) initialize();
   if(t != NULL){
      if(t->input != NULL) free(t->input);
      if(t->currToken != NULL) free(t->currToken);
      free(t);
   }
} /* delete_Tokenizer */


void Tokenizer_nextToken(Tokenizer *t){
   static const char fnName[] __attribute__((unused)) = "Tokenizer_nextToken";
   static Buffer *tokenBuffer = NULL;
   char          *input       = NULL;
   size_t        *currPos     = NULL;
   char          currChr      = '\0';
   
   if(!initialized) initialize();
   input   = t->input;
   currPos = &t->currPos;
   /* Initialize buffer if necessary */
   if(tokenBuffer == NULL){
      tokenBuffer = new_Buffer(INITIAL_SIZE);
   }
   else{
      Buffer_reset(tokenBuffer);
   }
   currChr = input[*currPos];
   if(currChr == '$'){
      getVariable(t, tokenBuffer);
   }
   else if(isdigit(currChr)){
      getNumber(t, tokenBuffer);
   }
   else if(currChr == '\"'){
      getString(t, tokenBuffer);
   }
   else if(currChr == '(' || currChr == ')' || currChr == ','){
      Buffer_appendChar(tokenBuffer, currChr);
      (*currPos)++;
   }
   else if(strchr(symbolChars, currChr) != NULL){
      getSymbol(t, tokenBuffer);
   }
   else if(isalpha(currChr)){
      getOperator(t, tokenBuffer);
   }
   else{
      fprintf(stderr, "FATAL: %s: Unexpected character \"%c\", code %d, encountered at %u.\n", 
            fnName, currChr, currChr, *currPos);
      exit(EXIT_FAILURE);
   }
   currChr = input[*currPos];
   if(isWhitespace(currChr)){
      eatWhitespace(t);
   }
   if(*currPos >= strlen(input)) t->hasMoreTokens = false;
   if(t->currToken != NULL) free(t->currToken);
   t->currToken = copyString(tokenBuffer->data);
} /* Tokenizer_nextToken */


bool Tokenizer_hasMoreTokens(Tokenizer *t){
   static const char fnName[] __attribute__((unused)) = "Tokenizer_hasMoreTokens()";
   
   if(!initialized) initialize();
   if(t != NULL){
      return t->hasMoreTokens;
   }
   else{
      fprintf(stderr, 
            "FATAL: %s: Undefined tokenizer.\n", fnName);
      exit(EXIT_FAILURE);
   }
} /* Tokenizer_hasMoreTokens */


const char *Tokenizer_getCurrentToken(Tokenizer *t){
   static const char fnName[] __attribute__((unused)) = "Tokenizer_getCurrentToken()";
   char *retVal = NULL;
   
   if(!initialized) initialize();
   if(t != NULL){
      retVal = t->currToken;
   }
   return retVal;
} /* Tokenizer_getCurrentToken */


/* -------------------------------------------------------------------
 * Local functions
 * ------------------------------------------------------------------- */

void eatWhitespace(Tokenizer *t){
   static const char fnName[] __attribute__((unused)) = "eatWhitespace()";
   char   *input   = t->input;
   size_t *currPos = &t->currPos;
   while(*currPos < strlen(input) && isWhitespace(input[*currPos])){
      (*currPos)++;
   }
} /* eatWhitespace */


void getNumber(Tokenizer *t, Buffer *b){
   static const char fnName[] __attribute__((unused)) = "getNumber()";
   char   *input   = t->input;
   size_t *currPos = &t->currPos;
   bool   gotDot   = false;
   char   currChr;
   
   currChr = input[*currPos];
   if(isdigit(currChr)){
      Buffer_appendChar(b, currChr);
      (*currPos)++;
      while(*currPos < strlen(input)){
         currChr = input[*currPos];
         if(!gotDot){
            if(currChr == '.'){
               gotDot = true;
            }
            else if(!isdigit(currChr)){
               break;
            }
         }
         else{
            if(!isdigit(currChr)){
               if(lastCharOf(b->data) == '.'){
                  Buffer_appendChar(b, '0');
               }
               break;
            }
         }
         Buffer_appendChar(b, currChr);
         (*currPos)++;
      }
   }
   else{
      fprintf(stderr, "FATAL: %s: Not a digit.\n", fnName);
      exit(EXIT_FAILURE);
   }
} /* Get number */


void getString(Tokenizer *t, Buffer *b){
   static const char fnName[] __attribute__((unused)) = "getString()";
   char   *input   = t->input;
   size_t *currPos = &t->currPos;
   bool   gotSlash = false;
   char   currChr;
   
   if(input[*currPos] == '\"'){
      Buffer_appendChar(b, '\"');
      (*currPos)++;
      while(*currPos < strlen(input)){
         currChr = input[*currPos];
         if(gotSlash){
            switch(currChr){
               case 'n':
                  currChr = '\n';
                  break;
               case 'r':
                  currChr = '\r';
                  break;
               case 't':
                  currChr = '\t';
                  break;
               default:
                  break;
            }
            Buffer_appendChar(b, currChr);
            gotSlash = false;
         }
         else{
            if(currChr == '\\'){
               gotSlash = true;
            }
            else{
               Buffer_appendChar(b, currChr);
               if(currChr == '\"'){
                  (*currPos)++;
                  break;
               }
            }
         }
         (*currPos)++;
      } /* while */
      if(lastCharOf(b->data) != '\"'){
         fprintf(stderr, "WARNING: %s: Missing quote.\n", fnName);
         Buffer_appendChar(b, '\"');
      }
   }
   else{
      fprintf(stderr, "FATAL: %s: Missing quote.\n", fnName);
      exit(EXIT_FAILURE);
   }
} /* Get string */


void getSymbol(Tokenizer *t, Buffer *b){
   static const char fnName[] __attribute__((unused)) = "getSymbol()";
   char   *input   = t->input;
   size_t *currPos = &t->currPos;
   char   currChr;
   
   currChr = input[*currPos];
   if(strchr(symbolChars, currChr) != NULL){
      Buffer_appendChar(b, currChr);
      (*currPos)++;
      while(*currPos < strlen(input)){
         currChr = input[*currPos];
         if(strchr(symbolChars, currChr) == NULL){
            break;
         }
         Buffer_appendChar(b, currChr);
         (*currPos)++;
      }
   }
   else{
      fprintf(stderr, "FATAL: %s: Illegal symbol.\n", fnName);
      exit(EXIT_FAILURE);
   }
} /* Get symbol */


void getOperator(Tokenizer *t, Buffer *b){
   static const char fnName[] __attribute__((unused)) = "getOperator()";
   char   *input   = t->input;
   size_t *currPos = &t->currPos;
   char   currChr;
   
   currChr = input[*currPos];
   if(isalpha(currChr)){
      Buffer_appendChar(b, currChr);
      (*currPos)++;
      while(*currPos < strlen(input)){
         currChr = input[*currPos];
         if(!isalnum(currChr) && currChr != '_' && currChr != '('){
            break;
         }
         Buffer_appendChar(b, currChr);
         (*currPos)++;
         if(currChr == '(') break;
      }
   }
   else{
      fprintf(stderr, "FATAL: %s: Illegal operator character: \"%c\".\n", fnName, currChr);
      exit(EXIT_FAILURE);
   }
} /* getOperator */


void getVariable(Tokenizer *t, Buffer *b){
   static const char fnName[] __attribute__((unused)) = "getVariable()";
   char   *input   = t->input;
   size_t *currPos = &t->currPos;
   char   currChr;

   currChr = input[*currPos];
   if(currChr == '$'){
      Buffer_appendChar(b, currChr);
      (*currPos)++;
      if(*currPos < strlen(input)){
         currChr = input[*currPos];
         if(isalpha(input[*currPos])){
            Buffer_appendChar(b, currChr);
            (*currPos)++;
            while(*currPos < strlen(input)){
               currChr = input[*currPos];
               if(!isalnum(currChr) && currChr != '_'){
                  break;
               }
               Buffer_appendChar(b, input[*currPos]);
               (*currPos)++;
            }
         }
      }
   }
   else{
      fprintf(stderr, "FATAL: %s: Illegal character in variable.\n", fnName);
      exit(EXIT_FAILURE);
   }
} /* getVariable */


void initialize(){
   static const char fnName[] __attribute__((unused)) = "initialize()";
   if(initialized) return;
   initialized     = true;
} /* initialize */

