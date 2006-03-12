/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Template.c,v 1.3 2006/03/12 01:08:03 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "astring.h"
#include "Template.h"
#include "bool.h"
#include "Buffer.h"
#include "List.h"
#include "Logging.h"
#include "stringext.h"

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

Statement *addStatement(Template *template, Buffer *cmd, Buffer *arg, char *fileName, int lineNo);
bool      commandExists(char *name);
void      debugPrintTemplate(Template *template, Statement *currStmt);
void      deleteStatement(Statement *st);
Command   *findCommand(char *name);
void      initialize(void);
void      registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)());

/* Standard BILE commands */
Action doComment(Template *t, char *cmd, char *param, FILE *op);
Action doEndIf(Template *t, char *cmd, FILE *op);
Action doFallback(Template *t, char *cmd, char *param, FILE *op);
Action doIf(Template *t, char *cmd, char *param, FILE *op);
Action doPrintDate(Template *t, char *cmd, char *param, FILE *op);
Action doPrintLiteral(Template *t, char *cmd, char *param, FILE *op);


/* -------------------------------------------------------------------
 * Local variables
 * ------------------------------------------------------------------- */

static List *commandList = NULL;
static bool initialized  = false;


/* -------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------- */

Template *Template_compile(char *fileName){
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
	template = (Template *)new_List();
	/* Initialise buffers */
	cmdBuffer = new_Buffer(128);
	/* Set the command buffer to a "print literal" command */
	Buffer_appendChar(cmdBuffer, '%');
	argBuffer = new_Buffer(512);
	
	if((fp = fopen(fileName, "r")) == NULL){
		Logging_fatalf("%s(): Cannot open template file \"%s\": %s.", __FUNCTION__, 
				fileName, strerror(errno));
	}
	else{
		while((currChr = fgetc(fp)) != EOF){
			/* Track line number so we can print it in error messages */
			if(currChr == '\n') lineNo++;
			switch(state){
				case 0:
				/* Add command to template */
				stmt = addStatement(template, cmdBuffer, argBuffer, fileName, lineNo);
				/* Check nesting is okay */
				if(stmt->type == ST_BEGIN)
					lastBlock = stmt->cmd;
				else if(stmt->type == ST_END){
					if(!strequalsi(stmt->cmd, lastBlock)){
						Logging_fatalf("%s(): File \"%s\", line %d: Unexpected end-of-block: /%s.", 
								__FUNCTION__, fileName, stmt->lineNo, stmt->cmd);
					}
				}
				Buffer_reset(cmdBuffer);
				Buffer_appendChar(cmdBuffer, '%');
				Buffer_reset(argBuffer);
				state = 1;
				/* Fall-through */
				case 1: /* Initial state */
				if(currChr == '[')
					state = 2;
				else
					Buffer_appendChar(argBuffer, currChr);
				break;

				case 2:
				if(currChr == '[')
					state = 3;
				else{
					Buffer_appendChar(argBuffer, '[');
					Buffer_appendChar(argBuffer, currChr);
					state = 1;
				}
				break;

				case 3:
				/* Add command to template */
				stmt = addStatement(template, cmdBuffer, argBuffer, fileName, lineNo);
				/* Check nesting is okay */
				if(stmt->type == ST_BEGIN)
					lastBlock = stmt->cmd;
				else if(stmt->type == ST_END){
					if(!strequalsi(stmt->cmd, lastBlock)){
						Logging_fatalf("%s(): File \"%s\", line %d: Unexpected end-of-block: /%s.", 
								__FUNCTION__, fileName, stmt->lineNo, stmt->cmd);
					}
				}
				Buffer_reset(cmdBuffer);
				Buffer_reset(argBuffer);
				state = 4;
				/* Fall-through */

				case 4:
				if(currChr == ' ')
					state = 5;
				else if(currChr == ']')
					state = 6;
				else
					Buffer_appendChar(cmdBuffer, currChr);
				break;

				case 5:
				if(currChr == ']')
					state = 6;
				else
					Buffer_appendChar(argBuffer, currChr);
				break;
				
				case 6:
				if(currChr == ']')
					state = 0;
				else{
					Buffer_appendChar(argBuffer, ']');
					Buffer_appendChar(argBuffer, currChr);
					state = 5;
				}
				break;
				default:
				/* Illegal state! */
				Logging_fatalf("%s(): Illegal state!  This can't happen!", __FUNCTION__);
			} /* switch(state) */
		} /* while(... != EOF) */
		fclose(fp);
		/* Append the last command if there is one */
		if(strlen(cmdBuffer->data) != 0){
			stmt = addStatement(template, cmdBuffer, argBuffer, fileName, lineNo);
			/* Check nesting is okay */
			if(stmt->type == ST_BEGIN)
				lastBlock = stmt->cmd;
			else if(stmt->type == ST_END){
				if(!strequalsi(stmt->cmd, lastBlock)){
					Logging_fatalf("%s(): File \"%s\", line %d: Unexpected end-of-block: /%s", 
							__FUNCTION__, fileName, stmt->lineNo, stmt->cmd);
				}
			}
		}
		delete_Buffer(cmdBuffer);
		delete_Buffer(argBuffer);
	}
	return template;
} /* Template_compile */


void Template_execute(Template *template, char *inputFile, FILE *op){
	Action    retVal;
	bool      keepGoing = true;
	Statement *currStmt = NULL;
	Command   *theCmd   = NULL;
	int       depth     = 0;
	
	if(!initialized) initialize();
	List_moveFirst((List *)template);
	while(keepGoing){
		/* Look up command and call its handler function */
		currStmt = (Statement *)(List_current((List *)template));
		switch(currStmt->type){
			case ST_SIMPLE:
			if(commandExists(currStmt->cmd)){
				theCmd = findCommand(currStmt->cmd);
				retVal = theCmd->begin(template, currStmt->cmd, currStmt->param, op);
			}
			else
				retVal = doFallback(template, currStmt->cmd, currStmt->param, op);
			break;
			
			case ST_BEGIN:
			case ST_END:
			if(commandExists(currStmt->cmd)){
				theCmd = findCommand(currStmt->cmd);
				if(currStmt->type == ST_BEGIN)
					retVal = theCmd->begin(template, currStmt->cmd, currStmt->param, op);
				else
					retVal = theCmd->end(template, currStmt->cmd, op);
			}
			else{
				/* Can't happen: Template_compile() should mark unrecognised 
				 * commands as ST_SIMPLE.
				 */
				if(currStmt->type == ST_BEGIN)
					Logging_fatalf("%s(): Illegal start-of-block command.", 
							__FUNCTION__);
				else
					Logging_fatalf("%s(): Illegal end-of-block command.", 
							__FUNCTION__);
			}
			break;
			
			default:
			/* Can't happen! */
			Logging_fatalf("%s(): Unknown command type.  This can't happen!", 
					__FUNCTION__);
		} /* switch(currStmt->type) */
		
		/* Process return code from handler function */
		switch(retVal){
			case ACTION_ABORT:
			keepGoing = false;
			break;
	
			case ACTION_CONTINUE:
			case ACTION_ENTER:
			keepGoing = List_moveNext((List *)template);
			break;
	
			case ACTION_REPEAT:
			if(theCmd->isBlock){
				/* Move back to the beginning of the block.
				 * We should really be using a stack here but templates are
				 * not meant to be that complicated... quicker to count blocks.
				 */
				if(!List_atStart((List *)template)){
					depth = 0;
					while(List_movePrevious((List *)template)){
						currStmt = (Statement *)(List_current((List *)template));
						/* Is it a beginning of block command? */
						if(currStmt->type == ST_BEGIN){
							/* Is it _our_ beginning of block command? */
							if(depth == 0) break;
							depth++;
						}
						else if(currStmt->type == ST_END)
							depth--;
					} /* while */
				}
				else{
					Logging_errorf("%s(): \"repeat block\" instruction at beginning of template", 
							__FUNCTION__);
					keepGoing = false;
				}
			}
			break;
	
			case ACTION_BREAK:
			/* Move forward until we clear the current block.
			 * Again, we should really be using a stack here...
			 */
			if(!List_atEnd((List *)template)){
				depth = 0;
				while(List_moveNext((List *)template)){
					currStmt = (Statement *)(List_current((List *)template));
					/* Is the current command an end-of-block command? */
					if(currStmt->type == ST_END){
						/* Is it _our_ end-of-block command? */
						if(depth == 0) break;
						depth--;
					}
					else if(currStmt->type == ST_BEGIN)
						depth++;
				} /* while */
			}
			else{
				Logging_errorf("%s(): premature end of template", __FUNCTION__);
				keepGoing = false;
			}
			break;
			
			default:
			/* Illegal state! */
			Logging_fatalf("%s(): Illegal return code.", __FUNCTION__);
		} /* switch(retVal) */
	} /* while(keepGoing) */
} /* Template_execute */


void delete_Template(Template *t){
   if(t != NULL){
	   delete_List((List *)t, true);
   }
   else{
	   Logging_warnf("%s: NULL argument", __FUNCTION__);
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
         fprintf(stderr, "\t%s\t%s\n", theCmd->name, 
		 		(theCmd->isBlock)? "Yes" : "No");
         pList = pList->next;
      }
      fprintf(stderr, "\n");
   }
} /* Bile_debugPrintCommands */


void Template_debugPrintTemplate(Template *template){
   debugPrintTemplate(template, NULL);
} /* Template_debugPrintTemplate */


void Bile_registerCommand(char *name, Action (*callback)()){
   registerCommand(name, false, callback, NULL);
} /* Bile_registerCommand */


void Bile_registerBlock(char *name, Action (*begin)(), Action (*end)()){
   registerCommand(name, true, begin, end);
} /* Bile_registerBlock */


/* -------------------------------------------------------------------
 * Local functions
 * ------------------------------------------------------------------- */

Statement *addStatement(Template *template, Buffer *cmd, Buffer *arg, char *fileName, int lineNo){
	Template      *pTpl        = NULL;
	bool          atEndOfBlock = false;
	Template      *newNode     = NULL;
	Statement     *newStmt     = NULL;
	StatementType type;
	char          *cmdName     = cmd->data;
	Command       *theCmd      = NULL;
	
	newStmt = (Statement *)malloc(sizeof(Statement));
	if(newStmt == NULL || newNode == NULL){
		Logging_fatalf("%s: Out of memory!", __FUNCTION__);
	}
	/* Check if end-of-block */
	if((atEndOfBlock = (cmdName[0] == '/'))) cmdName++;
	if(commandExists(cmdName)){
		theCmd = findCommand(cmdName);
		/* Check if end-of-block allowed */
		if(atEndOfBlock && !(theCmd->isBlock)){
			Logging_fatalf("%s: File \"%s\", line %d: \"%s\" is not a block command.", 
					__FUNCTION__, fileName, lineNo, cmdName);
		}
		if(atEndOfBlock)
			type = ST_END;
		else
			type = (theCmd->isBlock) ? ST_BEGIN : ST_SIMPLE;
	}
	else
		type = ST_SIMPLE;
	cmdName = cmd->data;
	if(type == ST_END && cmdName[0] == '/') cmdName++;
	newStmt->type   = type;
	newStmt->lineNo = lineNo;
	newStmt->cmd    = astrcpy(cmdName);
	newStmt->param  = astrcpy(arg->data);
	List_append((List *)template, newStmt);
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
      Logging_debug("TEMPLATE");
      Logging_debug("\t\tType\tCommand\tParam");
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
         Logging_debugf("\t%s\t", curr);
         Logging_debugf("\t%s\t", type);
         Logging_debugf("%s\t", stmt->cmd);
         Logging_debug("\"");
         for(ii = 0; ii < strlen(stmt->param); ++ii){
            currChr = (stmt->param)[ii];
            if(currChr == '\n'){
               Logging_debug("\\n");
            }
            else if(currChr == '\t'){
               Logging_debug("\\t");
            }
            else if(currChr == '\r'){
               Logging_debug("\\r");
            }
            else{
               Logging_debugf("%c", currChr);
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
	size_t ii;
	Command  *theCmd  = NULL;
	bool     cmdFound = false;
	
	if(!initialized) initialize();
	if(commandList != NULL){
		for(ii = 0; ii < List_length(commandList); ++ii){
			theCmd = (Command *)List_get(commandList, ii);
			if(strequals(theCmd->name, name)){
				cmdFound = true
				break;
			}
		}
	}
	else
		Logging_warnf("%s: No commands defined.", __FUNCTION__);
	return cmdFound ? theCmd : NULL;
} /* findCommand */


void initialize(void){
   if(initialized) return;
   initialized = true;
   /* Define the basic BILE commands */
   Bile_registerCommand("#", doComment);
   Bile_registerCommand("%", doPrintLiteral);
   Bile_registerCommand("DATE", doPrintDate);
   Bile_registerBlock("IF", doIf, doEndIf);
} /* initialize */


void registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)()){
   Command  *newCmd  = NULL;
   
   if(!initialized) initialize();
   if(commandExists(name))
      Logging_fatalf("%s: Command \"%s\"already exists!", __FUNCTION__, name);
   if((newCmd = (Command *)malloc(sizeof(Command))) == NULL)
      Logging_fatalf("%s: cannot allocate memory for command \"%s\".", 
	  		__FUNCTION__, name);
   newCmd->name    = name;
   newCmd->isBlock = isBlock;
   newCmd->begin   = begin;
   newCmd->end     = end;
   
   if(commandList == NULL)
      commandList = new_List();
   List_append(commandList, newCmd);
} /* registerCommand */


Action doComment(Template *t, char *cmd, char *param, FILE *op){
   return ACTION_CONTINUE;
} /* doComment */


Action doEndIf(Template *t, char *cmd, FILE *op){
   return ACTION_CONTINUE;
} /* doEndIf */


Action doFallback(Template *t, char *cmd, char *param, FILE *op){
   if((param == NULL) || strempty(param)){
      fprintf(op, "[[%s]]", cmd);
   }
   else{
      fprintf(op, "[[%s %s]]", cmd, param);
   }
   return ACTION_CONTINUE;
} /* doFallback */


Action doIf(Template *t, char *cmd, char *param, FILE *op){
   if(strequalsi(param, "TRUE")){
      return ACTION_ENTER;
   }
   else{
      return ACTION_BREAK;
   }
} /* doIf */


Action doPrintDate(Template *t, char *cmd, char *param, FILE *op){
   time_t t = time(NULL);
   fprintf(op, "%s", ctime(&t));
   return ACTION_CONTINUE;
} /* doPrintDate */


Action doPrintLiteral(Template *t, char *cmd, char *param, FILE *op){
   fprintf(op, "%s", param);
   return ACTION_CONTINUE;
} /* doPrintLiteral */

