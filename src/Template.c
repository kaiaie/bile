/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Template.c,v 1.6 2006/05/03 10:14:10 ken Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "Template.h"
#include "bool.h"
#include "Buffer.h"
#include "Expr.h"
#include "HtmlHandler.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "path.h"
#include "stringext.h"
#include "tokenize.h"
#include "Type.h"

/* -------------------------------------------------------------------
 * Local enums and structs
 * ------------------------------------------------------------------- */
 
typedef enum {ST_SIMPLE, ST_BEGIN, ST_END} StatementType;

/*
 * A Command holds the callback functions
 */
typedef struct _command{
   char   *name;
   bool   isBlock;
   Action (*begin)();
   Action (*end)();
} Command;

/*
 * A Statement is an instance of a Command within a Template
 */
typedef struct _statement{
   StatementType type;
   size_t        lineNo;
   char          *cmd;
   char          *param;
   void          *userData;
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
Action doBreak(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doComment(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doEndIf(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doFallback(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doIf(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doLetSet(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doPrintBody(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doPrintEscaped(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);
Action doPrintLiteral(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile);


/* -------------------------------------------------------------------
 * Local variables
 * ------------------------------------------------------------------- */

static List *commandList = NULL;
static bool initialized  = false;


/* -------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------- */
Template *new_Template(){
	Template *t = NULL;
	if(!initialized) initialize();
	t = (Template *)mu_malloc(sizeof(Template));
	t->timestamp  = 0;
	t->statements = new_List();
	return t;
}


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
	template = new_Template();
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
		template->fileName  = astrcpy(fileName);
		template->timestamp = getFileModificationTime(fileName);
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


void Template_execute(Template *template, Vars *v, char *inputFile, FILE *outputFile){
	Action    retVal;
	bool      keepGoing = true;
	Statement *currStmt = NULL;
	Command   *theCmd   = NULL;
	int       depth     = 0;
	
	if(!initialized) initialize();
	List_moveFirst(template->statements);
	while(keepGoing){
		/* Look up command and call its handler function */
		currStmt = (Statement *)(List_current(template->statements));
		switch(currStmt->type){
			case ST_SIMPLE:
			if(commandExists(currStmt->cmd)){
				theCmd = findCommand(currStmt->cmd);
				retVal = theCmd->begin(template, v, currStmt, inputFile, outputFile);
			}
			else
				retVal = doFallback(template, v, currStmt, inputFile, outputFile);
			break;
			
			case ST_BEGIN:
			case ST_END:
			if(commandExists(currStmt->cmd)){
				theCmd = findCommand(currStmt->cmd);
				if(currStmt->type == ST_BEGIN)
					retVal = theCmd->begin(template, v, currStmt, inputFile, outputFile);
				else
					retVal = theCmd->end(template, v, currStmt, inputFile, outputFile);
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
			keepGoing = List_moveNext(template->statements);
			break;
	
			case ACTION_REPEAT:
			if(theCmd->isBlock){
				/* Move back to the beginning of the block.
				 * We should really be using a stack here but templates are
				 * not meant to be that complicated... quicker to count blocks.
				 */
				if(!List_atStart(template->statements)){
					depth = 0;
					while(List_movePrevious(template->statements)){
						currStmt = (Statement *)(List_current(template->statements));
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
			if(!List_atEnd(template->statements)){
				depth = 0;
				while(List_moveNext(template->statements)){
					currStmt = (Statement *)(List_current(template->statements));
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
	   delete_List(t->statements, true);
	   mu_free(t->fileName);
	   mu_free(t);
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
	bool          atEndOfBlock = false;
	Statement     *newStmt     = NULL;
	StatementType type;
	char          *cmdName     = cmd->data;
	Command       *theCmd      = NULL;
	
	newStmt = (Statement *)mu_malloc(sizeof(Statement));
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
	newStmt->type     = type;
	newStmt->lineNo   = lineNo;
	newStmt->cmd      = astrcpy(cmdName);
	newStmt->param    = astrcpy(arg->data);
	newStmt->userData = NULL;
	List_append(template->statements, newStmt);
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
      for(ii = 0; ii < List_length(template->statements); ++ii){
         stmt = (Statement *)List_get(template->statements, ii);
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
      }
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


/* TODO: Move the command registration and command implementations to their own files */
Command *findCommand(char *name){
	size_t ii;
	Command  *theCmd  = NULL;
	bool     cmdFound = false;
	
	if(!initialized) initialize();
	if(commandList != NULL){
		for(ii = 0; ii < List_length(commandList); ++ii){
			theCmd = (Command *)List_get(commandList, ii);
			if(strequalsi(theCmd->name, name)){
				cmdFound = true;
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
   Bile_registerCommand("=", doPrintEscaped);
   Bile_registerCommand("BODY", doPrintBody);
   Bile_registerCommand("BREAK", doBreak);
   Bile_registerBlock("IF", doIf, doEndIf);
   Bile_registerCommand("LET", doLetSet);
   Bile_registerCommand("SET", doLetSet);
} /* initialize */


void registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)()){
   Command  *newCmd  = NULL;
   
   if(!initialized) initialize();
   if(commandExists(name))
      Logging_fatalf("%s: Command \"%s\"already exists!", __FUNCTION__, name);
   newCmd = (Command *)mu_malloc(sizeof(Command));
   newCmd->name    = name;
   newCmd->isBlock = isBlock;
   newCmd->begin   = begin;
   newCmd->end     = end;
   
   if(commandList == NULL)
      commandList = new_List();
   List_append(commandList, newCmd);
} /* registerCommand */


Action doBreak(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
	return ACTION_BREAK;
}

Action doComment(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
   return ACTION_CONTINUE;
} /* doComment */


Action doEndIf(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
   return ACTION_CONTINUE;
} /* doEndIf */


Action doFallback(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
   if((s->param == NULL) || strempty(s->param)){
      fprintf(outputFile, "[[%s]]", s->cmd);
   }
   else{
      fprintf(outputFile, "[[%s %s]]", s->cmd, s->param);
   }
   return ACTION_CONTINUE;
} /* doFallback */


Action doIf(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
	Action result;
	char *exprResult = NULL;
	Expr *e = NULL;
	
	if(s->userData == NULL){
		/* Tokenise and cache expression */
		s->userData = tokenize(s->param);
	}
	/* e = new_Expr2((List *)s->userData, v); */
	e = new_Expr(s->param, v);
	exprResult = Expr_evaluate(e);
	if(Type_toBool(exprResult)){
	  result = ACTION_ENTER;
	}
	else{
	  result = ACTION_BREAK;
	}
	mu_free(exprResult);
	delete_Expr(e);
	return result;
} /* doIf */


Action doLetSet(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
	List *tokens = NULL;
	char *varName = NULL;
	Expr *e = NULL;
	char *exprResult = NULL;

	tokens = tokenize(s->param);
	if(List_length(tokens) > 2 && 
		((char *)List_get(tokens, 0))[0] == '$' && 
		strequals((char *)List_get(tokens, 1), "=")){
		varName = (char *)List_get(tokens, 0);
		varName = astrcpy(&varName[1]);
		List_remove(tokens, 0, true);
		List_remove(tokens, 0, true);
		e = new_Expr2(tokens, v);
		exprResult = Expr_evaluate(e);
		if(strequalsi(s->cmd, "LET"))
			Vars_let(v, varName, exprResult);
		else
			Vars_set(v, varName, exprResult);
		mu_free(varName);
	}
	else{
		Logging_warnf("Syntax error in template file \"%s\", line %d.", 
			t->fileName, s->lineNo);
	}
	return ACTION_CONTINUE;
} /* doLet */


Action doPrintBody(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
	htmlWriteOutput(inputFile, WF_HTMLBODY, outputFile);
	return ACTION_CONTINUE;
} /* doPrintBody */


Action doPrintEscaped(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
	char *exprResult = NULL;
	size_t ii;
	char currChar;
	Expr *e = NULL;
	
	if(s->userData == NULL){
		/* Tokenise and cache expression */
		s->userData = tokenize(s->param);
	}
	/* e = new_Expr2((List *)s->userData, v); */
	e = new_Expr(s->param, v);
	exprResult = Expr_evaluate(e);
	for(ii = 0; ii < strlen(exprResult); ++ii){
		currChar = exprResult[ii];
		switch(currChar){
			case '&': fprintf(outputFile, "&amp;"); break;
			case '<': fprintf(outputFile, "&lt;"); break;
			case '>': fprintf(outputFile, "&gt;"); break;
			case '"': fprintf(outputFile, "&quot;"); break;
			default: fprintf(outputFile, "%c", currChar); break;
		}
	}
	mu_free(exprResult);
	delete_Expr(e);
	return ACTION_CONTINUE;
} /* doPrintLiteral */


Action doPrintLiteral(Template *t, Vars *v, Statement *s, char *inputFile, FILE *outputFile){
	fprintf(outputFile, "%s", s->param);
	return ACTION_CONTINUE;
} /* doPrintLiteral */


