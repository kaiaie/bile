#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bool.h"
#include "Buffer.h"
#include "List.h"
#include "Template.h"
#include "util.h"

/* -------------------------------------------------------------------
 * Local enums and structs
 * ------------------------------------------------------------------- */
 
typedef enum {ST_SIMPLE, ST_BEGIN, ST_END} StatementType;

typedef struct _command{
   char   *name;
   bool   isBlock;
   Action (*begin)();
   Action (*end)();
} Command;

typedef struct _statement{
   StatementType type;
   size_t        lineNo;
   char          *cmd;
   char          *param;
} Statement;


/* -------------------------------------------------------------------
 * Local function declarations
 * ------------------------------------------------------------------- */

Statement *addStatement(Template **template, Buffer *cmd, Buffer *arg, char *fileName, int lineNo);
bool      commandExists(char *name);
void      debugPrintTemplate(Template *template, Statement *currStmt);
void      deleteStatement(Statement *st);
Command   *findCommand(char *name);
void      initialize(void);
void      registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)());

/* Standard BILE commands */
Action doComment(char *cmd, char *param, FILE *op);
Action doEndIf(char *cmd, FILE *op);
Action doFallback(char *cmd, char *param, FILE *op);
Action doIf(char *cmd, char *param, FILE *op);
Action doPrintDate(char *cmd, char *param, FILE *op);
Action doPrintLiteral(char *cmd, char *param, FILE *op);


/* -------------------------------------------------------------------
 * Local variables
 * ------------------------------------------------------------------- */

static List *commandList = NULL;
static bool initialized  = false;


/* -------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------- */

Template *Template_compile(char *fileName){
   static const char fnName[] __attribute__((unused)) = "Template_compile()";   
   int       currChr;
   FILE      *fp        = NULL;
   int       state      = 1;
   int       lineNo     = 1;
   Template  *template  = NULL;
   Buffer    *cmdBuffer = NULL;
   Buffer    *argBuffer = NULL;
   Statement *stmt      = NULL;
   char      *lastBlock = NULL;
   
   if(!initialized) initialize();
   /* Initialise buffers */
   cmdBuffer = new_Buffer(128);
   /* Set the command buffer to a "print literal" command */
   Buffer_appendChar(cmdBuffer, '%');
   argBuffer = new_Buffer(512);

   if((fp = fopen(fileName, "r")) == NULL){
      fprintf(stderr, "FATAL: %s: Cannot open template file %s.\n", fnName, fileName);
   }
   else{
      while((currChr = fgetc(fp)) != EOF){
         if(currChr == '\n'){
            lineNo++;
         }
         switch(state){
            case 0:
               /* Add command to template */
               stmt = addStatement(&template, cmdBuffer, argBuffer, fileName, lineNo);
               /* Check nesting is okay */
               if(stmt->type == ST_BEGIN){
                  lastBlock = stmt->cmd;
               }
               else if(stmt->type == ST_END){
                  if(strcmpi(stmt->cmd, lastBlock) != 0){
                     fprintf(stderr, "FATAL: %s: File %s, line %d: Unexpected end-of-block: /%s.\n", 
                           fnName, fileName, stmt->lineNo, stmt->cmd);
                     debugPrintTemplate(template, stmt);
                     fprintf(stderr, "Last block command: %s.\n", lastBlock);
                     exit(EXIT_FAILURE);
                  }
               }
               Buffer_reset(cmdBuffer);
               Buffer_appendChar(cmdBuffer, '%');
               Buffer_reset(argBuffer);
               state = 1;
               /* Fall-through */
            case 1: /* Initial state */
               if(currChr == '['){
                  state = 2;
               }
               else{
                  Buffer_appendChar(argBuffer, currChr);
               }
               break;
            case 2:
               if(currChr == '['){
                  state = 3;
               }
               else{
                  Buffer_appendChar(argBuffer, '[');
                  Buffer_appendChar(argBuffer, currChr);
                  state = 1;
               }
               break;
            case 3:
               /* Add command to template */
               stmt = addStatement(&template, cmdBuffer, argBuffer, fileName, lineNo);
               /* Check nesting is okay */
               if(stmt->type == ST_BEGIN){
                  lastBlock = stmt->cmd;
               }
               else if(stmt->type == ST_END){
                  if(strcmpi(stmt->cmd, lastBlock) != 0){
                     fprintf(stderr, "FATAL: %s: File %s, line %d: Unexpected end-of-block: /%s.\n", 
                           fnName, fileName, stmt->lineNo, stmt->cmd);
                     debugPrintTemplate(template, stmt);
                     fprintf(stderr, "Last block command: %s.\n", lastBlock);
                     exit(EXIT_FAILURE);
                  }
               }
               Buffer_reset(cmdBuffer);
               Buffer_reset(argBuffer);
               state = 4;
               /* Fall-through */
            case 4:
               if(currChr == ' '){
                  state = 5;
               }
               else if(currChr == ']'){
                  state = 6;
               }
               else{
                  Buffer_appendChar(cmdBuffer, currChr);
               }
               break;
            case 5:
               if(currChr == ']'){
                  state = 6;
               }
               else{
                  Buffer_appendChar(argBuffer, currChr);
               }
               break;
            case 6:
               if(currChr == ']'){
                  state = 0;
               }
               else{
                  Buffer_appendChar(argBuffer, ']');
                  Buffer_appendChar(argBuffer, currChr);
                  state = 5;
               }
               break;
            default:
               /* Illegal state! */
               fprintf(stderr, "FATAL: %s: Illegal state!\n", fnName);
         } /* switch(state) */
      } /* while(... != EOF) */
      fclose(fp);
      /* Append the last command if there is one */
      if(strcmp(cmdBuffer->data, "") != 0){
         stmt = addStatement(&template, cmdBuffer, argBuffer, fileName, lineNo);
         /* Check nesting is okay */
         if(stmt->type == ST_BEGIN){
            lastBlock = stmt->cmd;
         }
         else if(stmt->type == ST_END){
            if(strcmp(stmt->cmd, lastBlock) != 0){
               fprintf(stderr, "FATAL: %s: File %s, line %d: Unexpected end-of-block: /%s.\n", 
                     fnName, fileName, stmt->lineNo, stmt->cmd);
               debugPrintTemplate(template, stmt);
               fprintf(stderr, "Last block command: %s.\n", lastBlock);
               exit(EXIT_FAILURE);
            }
         }
      }
      delete_Buffer(cmdBuffer);
      delete_Buffer(argBuffer);
   }
   return template;
} /* Template_compile */


void Template_execute(Template *template, FILE *op){
   static const char fnName[] __attribute__((unused)) = "Template_execute()";
   Action    retVal;
   bool      keepGoing = true;
   Template  *currCmd  = template;
   Statement *currStmt = NULL;
   Command   *theCmd   = NULL;
   int       depth     = 0;
   
   if(!initialized) initialize();
   while(keepGoing){
      /* Look up command */
      currStmt = (Statement *)(currCmd->data);
      switch(currStmt->type){
         case ST_SIMPLE:
            if(commandExists(currStmt->cmd)){
               theCmd = findCommand(currStmt->cmd);
               retVal = theCmd->begin(currStmt->cmd, currStmt->param, op);
            }
            else{
               retVal = doFallback(currStmt->cmd, currStmt->param, op);
            }
            break;
         case ST_BEGIN:
         case ST_END:
            if(commandExists(currStmt->cmd)){
               theCmd = findCommand(currStmt->cmd);
               if(currStmt->type == ST_BEGIN){
                  retVal = theCmd->begin(currStmt->cmd, currStmt->param, op);
               }
               else{
                  retVal = theCmd->end(currStmt->cmd, op);
               }
            }
            else{
               /* Can't happen: Template_compile() should mark unrecognised 
                  commands as ST_SIMPLE.
                */
               if(currStmt->type == ST_BEGIN){
                  fprintf(stderr, "FATAL: %s: Illegal start-of-block command.\n", fnName);
               }
               else{
                  fprintf(stderr, "FATAL: %s: Illegal end-of-block command.\n", fnName);
               }
               exit(EXIT_FAILURE);
            }
            break;
         default:
            /* Can't happen! */
            fprintf(stderr, "FATAL: %s: Unknown command type.\n", fnName);
            exit(EXIT_FAILURE);
      } /* (currStmt->type) */
      switch(retVal){
         case ACTION_ABORT:
            keepGoing = false;
            break;
         case ACTION_CONTINUE:
            if(currCmd->next != NULL){
               currCmd   = currCmd->next;
               keepGoing = true;
            }
            else{
               keepGoing = false;
            }
            break;
         case ACTION_ENTER:
            if(currCmd->next != NULL){
               currCmd   = currCmd->next;
               keepGoing = true;
            }
            else{
               keepGoing = false;
            }
            break;
         case ACTION_REPEAT:
            if(theCmd->isBlock){
               /* Move back to the beginning of the block. */
               if(currCmd->prev != NULL){
                  depth = 0;
                  while(true){
                     currStmt = (Statement *)(currCmd->data);
                     /* Is it a beginning of block command? */
                     if(currStmt->type == ST_BEGIN){
                        /* Is it *our* beginning of block command? */
                        if(depth == 0){
                           break;
                        }
                        else{
                           depth++;
                        }
                     }
                     else if(currStmt->type == ST_END){
                        depth--;
                     }
                     if(currCmd->prev == NULL){
                        /* Error: "repeat block" instruction outside a block. */
                        keepGoing = false;
                        break;
                     }
                     else{
                        currCmd = currCmd->prev;
                     }
                  } /* while(true) */
               }
               else{
                  /* Error: "repeat block" instruction at beginning of template */
                  keepGoing = false;
               }
            }
            break;
         case ACTION_BREAK:
            /* Move forward until we clear the current block. */
            if(currCmd->next != NULL){
               currCmd = currCmd->next;
               depth = 0;
               while(true){
                  currStmt = (Statement *)(currCmd->data);
                  /* Is the current command an end-of-block command? */
                  if(currStmt->type == ST_END){
                     /* Is it OUR end-of-block command? */
                     if(depth == 0){
                        break;
                     }
                     else{
                        depth--;
                     }
                  }
                  else if(currStmt->type == ST_BEGIN){
                     depth++;
                  }
                  if(currCmd->next == NULL){
                     /* Error: break command outside a block */
                     keepGoing = false;
                     break;
                  }
                  else{
                     currCmd = currCmd->next;
                  }
               } /* while(true) */
            }
            else{
               /* Error: premature end of template */
               keepGoing = false;
            }
            break;
         default:
            /* Illegal state! */
            fprintf(stderr, "FATAL: %s: Illegal return code.\n", fnName);
            exit(EXIT_FAILURE);
      } /* switch(retVal) */
   } /* while(keepGoing) */
} /* Template_execute */


void delete_Template(Template *t){
   if(t != NULL){
      if(t->next != NULL){
         delete_Template(t->next);
      }
      deleteStatement((Statement *)t->data);
      if(t->prev != NULL){
         (t->prev)->next = NULL;
      }
      free(t);
   }
} /* delete_Template */


void Bile_debugPrintCommands(){
   ListNode *pList  = NULL;
   Command  *theCmd = NULL;
   
   if(commandList != NULL){
      pList = commandList->first;
      fprintf(stderr, "COMMAND TABLE\n");
      fprintf(stderr, "\tCommand\tBlock?\n");
      while(pList != NULL){
         theCmd = (Command *)pList->data;
         fprintf(stderr, "\t%s\t%s\n", theCmd->name, (theCmd->isBlock)? "Yes" : "No");
         pList = pList->next;
      }
      fprintf(stderr, "\n");
   }
} /* Bile_debugPrintCommands */


void Template_debugPrintTemplate(Template *template){
   debugPrintTemplate(template, NULL);
} /* Template_debugPrintTemplate */


void Bile_registerCommand (char *name, Action (*callback)()){
   registerCommand(name, false, callback, NULL);
} /* Bile_registerCommand */


void Bile_registerBlock(char *name, Action (*begin)(), Action (*end)()){
   registerCommand(name, true, begin, end);
} /* Bile_registerBlock */


/* -------------------------------------------------------------------
 * Local functions
 * ------------------------------------------------------------------- */

Statement *addStatement(Template **template, Buffer *cmd, Buffer *arg, char *fileName, int lineNo){
   static const char fnName[] __attribute__((unused)) = "addStatement()";
   Template      *pTpl        = NULL;
   bool          atEndOfBlock = false;
   Template      *newNode     = NULL;
   Statement     *newStmt     = NULL;
   StatementType type;
   char          *cmdName     = cmd->data;
   Command       *theCmd      = NULL;
   
   newStmt = (Statement *)malloc(sizeof(Statement));
   newNode = (Template *)malloc(sizeof(Template));
   if(newStmt == NULL || newNode == NULL){
      fprintf(stderr, "FATAL: %s: Out of memory!\n", fnName);
      exit(EXIT_FAILURE);
   }
   /* Check if end-of-block */
   if((atEndOfBlock = (cmdName[0] == '/'))) cmdName++;
   if(commandExists(cmdName)){
      theCmd = findCommand(cmdName);
      /* Check if end-of-block allowed */
      if(atEndOfBlock && !(theCmd->isBlock)){
         fprintf(stderr, "FATAL: %s: File %s, line %d: %s is not a block command.\n", 
            fnName, fileName, lineNo, cmdName);
         exit(EXIT_FAILURE);
      }
      if(atEndOfBlock){
         type = ST_END;
      }
      else{
         if(theCmd->isBlock){
            type = ST_BEGIN;
         }
         else{
            type = ST_SIMPLE;
         }
      }
   }
   else{
      type = ST_SIMPLE;
   }
   cmdName = cmd->data;
   if(type == ST_END && cmdName[0] == '/') cmdName++;
   newStmt->type    = type;
   newStmt->lineNo  = lineNo;
   newStmt->cmd     = copyString(cmdName);
   newStmt->param   = copyString(arg->data);
   
   /* Initialise node */
   newNode->data = newStmt;
   newNode->next = NULL;
   
   /* Attach node */
   if(*template == NULL){
      newNode->prev = NULL;
      *template = newNode;
   }
   else{
      pTpl = *template;
      while(pTpl->next != NULL){
         pTpl = pTpl->next;
      }
      pTpl->next = newNode;
      newNode->prev = pTpl;
   }
   return newStmt;
}


bool commandExists(char *name){
   if(!initialized) initialize();

   return (findCommand(name) != NULL);
} /* commandExists */


void debugPrintTemplate(Template *template, Statement *currStmt){
   Template  *pTpl   = NULL;
   Statement *stmt   = NULL;
   char      type[2] = "x";
   char      curr[2] = " ";
   char      currChr;
   size_t    ii;
   
   if(!initialized) initialize();
   if(template != NULL){
      fprintf(stderr, "TEMPLATE\n");
      fprintf(stderr, "\t\tType\tCommand\tParam\n");
      pTpl = template;
      do{
         stmt = (Statement *)pTpl->data;
         if(currStmt != NULL){
            if(currStmt == stmt){
               curr[0] = '>';
            }
            else{
               curr[0] = ' ';
            }
         }
         else{
            curr[0] = ' ';
         }
         if(stmt->type == ST_SIMPLE){
            type[0] = 'S';
         }
         else if(stmt->type == ST_BEGIN){
            type[0] = 'B';
         }
         else{
            type[0] = 'E';
         }
         fprintf(stderr, "\t%s\t", curr);
         fprintf(stderr, "\t%s\t", type);
         fprintf(stderr, "%s\t", stmt->cmd);
         fprintf(stderr, "\"");
         for(ii = 0; ii < strlen(stmt->param); ++ii){
            currChr = (stmt->param)[ii];
            if(currChr == '\n'){
               fprintf(stderr, "\\n");
            }
            else if(currChr == '\t'){
               fprintf(stderr, "\\t");
            }
            else if(currChr == '\r'){
               fprintf(stderr, "\\r");
            }
            else{
               fprintf(stderr, "%c", currChr);
            }
         }
         fprintf(stderr, "\"\n");
         pTpl = pTpl->next;
      } while(pTpl != NULL);
      fprintf(stderr, "\n");
   }
} /* debugPrintTemplate */


void deleteStatement(Statement *st){
   if(st != NULL){
      if(st->cmd != NULL) free(st->cmd);
      if(st->param != NULL) free(st->param);
      free(st);
   }
} /* deleteStatement */


Command *findCommand(char *name){
   ListNode *pCmd    = NULL;
   Command  *theCmd  = NULL;
   bool     cmdFound = false;
   
   if(!initialized) initialize();
   if(commandList != NULL){
      pCmd = commandList->first;
      while(pCmd != NULL){
         theCmd = (Command *)(pCmd->data);
         if(strcmpi(name, theCmd->name) == 0){
            cmdFound = true;
            break;
         }
         pCmd = pCmd->next;
      }
      if(!cmdFound) theCmd = NULL;
   }
   return theCmd;
} /* findCommand */


void initialize(void){
   static const char fnName[] __attribute__((unused)) = "initialize()";
   if(initialized) return;
   initialized = true;
   /* Define the basic BILE commands */
   Bile_registerCommand("#", doComment);
   Bile_registerCommand("%", doPrintLiteral);
   Bile_registerCommand("DATE", doPrintDate);
   Bile_registerBlock("IF", doIf, doEndIf);
} /* initialize */


void registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)()){
   static const char fnName[] __attribute__((unused)) = "registerCommand()";
   Command  *newCmd  = NULL;
   
   if(!initialized) initialize();
   if(commandExists(name)){
      fprintf(stderr, "FATAL: %s: Command \"%s\"already exists!\n", 
            fnName, name);
      exit(EXIT_FAILURE);
   }
   
   newCmd = (Command *)malloc(sizeof(Command));
   if(newCmd == NULL){
      fprintf(stderr, "FATAL: %s: cannot allocate memory for command.\n", fnName);
      exit(EXIT_FAILURE);
   }
   newCmd->name    = name;
   newCmd->isBlock = isBlock;
   newCmd->begin   = begin;
   newCmd->end     = end;
   
   if(commandList == NULL){
      commandList = new_List();
   }
   List_append(commandList, newCmd);
} /* registerCommand */


Action doComment(char *cmd, char *param, FILE *op){
   return ACTION_CONTINUE;
} /* doComment */


Action doEndIf(char *cmd, FILE *op){
   return ACTION_CONTINUE;
} /* doEndIf */


Action doFallback(char *cmd, char *param, FILE *op){
   if((param == NULL) || (strcmp(param, "") == 0)){
      fprintf(op, "[[%s]]", cmd);
   }
   else{
      fprintf(op, "[[%s %s]]", cmd, param);
   }
   return ACTION_CONTINUE;
} /* doFallback */


Action doIf(char *cmd, char *param, FILE *op){
   if(strcmpi(param, "TRUE") == 0){
      return ACTION_ENTER;
   }
   else{
      return ACTION_BREAK;
   }
} /* doIf */


Action doPrintDate(char *cmd, char *param, FILE *op){
   time_t t = time(NULL);
   fprintf(op, "%s", ctime(&t));
   return ACTION_CONTINUE;
} /* doPrintDate */


Action doPrintLiteral(char *cmd, char *param, FILE *op){
   fprintf(op, "%s", param);
   return ACTION_CONTINUE;
} /* doPrintLiteral */

