/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Template.c,v 1.26 2010/08/31 15:11:58 ken Exp $
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "Template.h"
#include "Publication.h"
#include "Tags.h"
#include "Buffer.h"
#include "Command.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "path.h"
#include "stringext.h"

extern Publication *thePublication;

/* 
** Local function declarations
*/
bool      printEscapedHtml(const char *s, FILE *output);
Statement *addStatement(Template *template, char *cmd, char *arg, char *fileName, int lineNo);
void      debugPrintTemplate(Template *template, Statement *currStmt);
void      deleteStatement(Statement *st);


/*
** Public functions
*/

/** Allocates and initialises a new Template structure */
Template *new_Template(){
	Template *t = NULL;
	t = (Template *)mu_malloc(sizeof(Template));
	t->timestamp  = 0;
	t->statements = new_List();
	t->context = NULL;
	t->variables = NULL;
	t->inputFile = NULL;
	t->outputFile = NULL;
	t->outputFileName = NULL;
	return t;
}


/** Parses a template file and converts it into a list of Statements for 
*** Template_execute() to process
**/
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
	Command   *immediate = NULL;
	char      *immediateArgs[2];
	
	template = new_Template();
	/* Initialise buffers */
	cmdBuffer = new_Buffer(128);
	/* Set the command buffer to a "print literal" command */
	Buffer_appendChar(cmdBuffer, '%');
	argBuffer = new_Buffer(512);
	
	if ((fp = fopen(fileName, "r")) == NULL) {
		Logging_fatalf("%s(): Cannot open template file \"%s\": %s.", __FUNCTION__, 
				fileName, strerror(errno));
	}
	else {
		template->fileName  = astrcpy(fileName);
		template->timestamp = getFileModificationTime(fileName);
		while ((currChr = fgetc(fp)) != EOF) {
			/* Track line number so we can print it in error messages */
			if (currChr == '\n') lineNo++;
			switch (state) {
				case 0:
				/* If command prefixed with a "!" character, then execute immediately,
				** passing command and args in the context field
				*/
				if (cmdBuffer->data[0] == '!') {
					if ((immediate = Command_find(cmdBuffer->data)) != NULL) {
						immediateArgs[0] = cmdBuffer->data;
						immediateArgs[1] = argBuffer->data;
						template->context = immediateArgs;
						if (immediate->begin(template) == ACTION_ABORT) {
							Logging_fatalf("Error processing command");
						}
						template->context = NULL;
					}
					else {
						Logging_warnf("Unknown command");
					}
				}
				else {
					/* Add command to template */
					stmt = addStatement(template, cmdBuffer->data, argBuffer->data, fileName, lineNo);
				}
				Buffer_reset(cmdBuffer);
				Buffer_appendChar(cmdBuffer, '%');
				Buffer_reset(argBuffer);
				state = 1;
				/* Fall-through */
				case 1: /* Initial state */
				if (currChr == '[') {
					state = 2;
				}
				else {
					Buffer_appendChar(argBuffer, currChr);
				}
				break;

				case 2:
				if (currChr == '[') {
					state = 3;
				}
				else {
					Buffer_appendChar(argBuffer, '[');
					Buffer_appendChar(argBuffer, currChr);
					state = 1;
				}
				break;

				case 3:
				/* Add command to template */
				stmt = addStatement(template, cmdBuffer->data, argBuffer->data, fileName, lineNo);
				/* Check nesting is okay */
				if (stmt->type == ST_BEGIN) {
					lastBlock = stmt->cmd;
				}
				else if (stmt->type == ST_END) {
					if (!strxequalsi(stmt->cmd, lastBlock)) {
						Logging_fatalf("%s(): File \"%s\", line %d: Unexpected end-of-block: /%s.", 
								__FUNCTION__, fileName, stmt->lineNo, stmt->cmd);
					}
				}
				Buffer_reset(cmdBuffer);
				Buffer_reset(argBuffer);
				state = 4;
				/* Fall-through */

				case 4:
				if(currChr == ' ') {
					state = 5;
				}
				else if (currChr == ']') {
					state = 6;
				}
				else {
					Buffer_appendChar(cmdBuffer, currChr);
				}
				break;

				case 5:
				if (currChr == ']') {
					state = 6;
				}
				else {
					Buffer_appendChar(argBuffer, currChr);
				}
				break;
				
				case 6:
				if (currChr == ']') {
					state = 0;
				}
				else {
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
		if (!strxempty(cmdBuffer->data)) {
			stmt = addStatement(template, cmdBuffer->data, argBuffer->data, fileName, lineNo);
		}
		delete_Buffer(cmdBuffer);
		delete_Buffer(argBuffer);
	}
	return template;
} /* Template_compile */

/** Executes the commands in the Template, writing the output to the specified 
*** file
**/
void Template_execute(Template *template, void *context, char *outputFileName) {
	Action      retVal;
	bool        keepGoing = true;
	Statement   *currStmt = NULL;
	Command     *theCmd   = NULL;
	int         depth     = 0;
	BileObjType templateType = *((BileObjType *)context);
	
	template->context        = context;
	template->outputFileName = outputFileName;
	if (strxnullorempty(outputFileName)) {
		template->outputFile = stdout;
	}
	else {
		template->outputFile = fopen(outputFileName, "w");
	}
	if (template->outputFile == NULL) {
		Logging_fatalf("Unable to open template output file \"%s\": %s", 
			outputFileName, strerror(errno));
	}
	if (templateType == BILE_STORY) {
		template->variables = ((Story *)context)->variables;
		template->inputFile = ((Story *)context)->inputPath;
	}
	else if (templateType == BILE_INDEX) {
		template->variables = ((Index *)context)->variables;
		template->inputFile = NULL;
	}
	else if (templateType == BILE_TAGS) {
		template->variables = ((Tags *)context)->variables;
		template->inputFile = NULL;
	}
	else {
		Logging_fatal("Unsupported template type.");
	}
	List_moveFirst(template->statements);
	while (keepGoing) {
		/* Look up command and call its handler function */
		currStmt = (Statement *)(List_current(template->statements));
		switch (currStmt->type) {
			case ST_SIMPLE:
			if (Command_exists(currStmt->cmd)) {
				theCmd = Command_find(currStmt->cmd);
				retVal = theCmd->begin(template);
			}
			else {
				retVal = Command_doFallback(template);
			}
			break;
			
			case ST_BEGIN:
			case ST_END:
			if (Command_exists(currStmt->cmd)) {
				theCmd = Command_find(currStmt->cmd);
				if (currStmt->type == ST_BEGIN) {
					retVal = theCmd->begin(template);
				}
				else {
					retVal = theCmd->end(template);
				}
			}
			else {
				/* Can't happen: Template_compile() should mark unrecognised 
				** commands as ST_SIMPLE.
				**/
				if (currStmt->type == ST_BEGIN) {
					Logging_fatalf(
						"%s(): Illegal start-of-block command.", 
						__FUNCTION__
					);
				}
				else {
					Logging_fatalf(
						"%s(): Illegal end-of-block command.", 
						__FUNCTION__
					);
				}
			}
			break;
			
			default:
			/* Can't happen! */
			Logging_fatalf(
				"%s(): Unknown command type.  This can't happen!", 
				__FUNCTION__
			);
		} /* switch(currStmt->type) */
		
		/* Process return code from handler function */
		switch (retVal) {
			case ACTION_ABORT:
			keepGoing = false;
			break;
	
			case ACTION_CONTINUE:
			case ACTION_ENTER:
			keepGoing = List_moveNext(template->statements);
			break;
	
			case ACTION_REPEAT:
			if (theCmd->isBlock) {
				/* Move back to the beginning of the block.
				 * We should really be using a stack here but templates are
				 * not meant to be that complicated... quicker to count blocks.
				 */
				if (!List_atStart(template->statements)) {
					depth = 0;
					while (List_movePrevious(template->statements)) {
						currStmt = (Statement *)(List_current(template->statements));
						/* Is it a beginning of block command? */
						if (currStmt->type == ST_BEGIN) {
							/* Is it _our_ beginning of block command? */
							if (depth == 0) break;
							depth++;
						}
						else if (currStmt->type == ST_END) {
							depth--;
						}
					} /* while */
				}
				else {
					Logging_errorf(
						"%s(): \"repeat block\" instruction at beginning of template", 
						__FUNCTION__
					);
					keepGoing = false;
				}
			}
			break;
	
			case ACTION_BREAK:
			/* Move forward until we clear the current block.
			 * Again, we should really be using a stack here...
			 */
			if (!List_atEnd(template->statements)) {
				depth = 0;
				while (List_moveNext(template->statements)) {
					currStmt = (Statement *)(List_current(template->statements));
					/* Is the current command an end-of-block command? */
					if (currStmt->type == ST_END) {
						/* Is it _our_ end-of-block command? */
						if (depth == 0) {
							currStmt->broken = true;
							break;
						}
						depth--;
					}
					else if (currStmt->type == ST_BEGIN) {
						depth++;
					}
				} /* while */
			}
			else {
				Logging_errorf("%s(): premature end of template", __FUNCTION__);
				keepGoing = false;
			}
			break;
			
			default:
			/* Illegal state! */
			Logging_fatalf("%s(): Illegal return code.", __FUNCTION__);
		} /* switch(retVal) */
	} /* while(keepGoing) */
	if (template->outputFile != stdout) fclose(template->outputFile);
	template->outputFileName = NULL;
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


void Template_debugPrintTemplate(Template *template){
   debugPrintTemplate(template, NULL);
} /* Template_debugPrintTemplate */


/* -------------------------------------------------------------------
 * Local functions
 * ------------------------------------------------------------------- */

Statement *addStatement(Template *template, char *cmd, char *arg, char *fileName, int lineNo){
	bool          atEndOfBlock = false;
	Statement     *newStmt     = NULL;
	StatementType type;
	char          *cmdName     = cmd;
	Command       *theCmd      = NULL;
	
	newStmt = (Statement *)mu_malloc(sizeof(Statement));
	/* Check if end-of-block */
	if((atEndOfBlock = (cmdName[0] == '/'))) cmdName++;
	if(Command_exists(cmdName)){
		theCmd = Command_find(cmdName);
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
	if(type == ST_END && cmdName[0] == '/') cmdName++;
	newStmt->type     = type;
	newStmt->lineNo   = lineNo;
	newStmt->cmd      = astrcpy(cmdName);
	newStmt->param    = astrcpy(arg);
	newStmt->userData = NULL;
	newStmt->broken   = false;
	List_append(template->statements, newStmt);
	/* Force timestamp if the command is "dirty" */
	if(theCmd->isDirty) template->timestamp = 2147483647L;
	return newStmt;
}


void debugPrintTemplate(Template *template, Statement *currStmt){
   Template  *pTpl   = NULL;
   Statement *stmt   = NULL;
   char      type[2] = "x";
   char      curr[2] = " ";
   char      currChr;
   size_t    ii;
   
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
      mu_free(st->cmd);
      mu_free(st->param);
      mu_free(st);
   }
} /* deleteStatement */


/* Template_findMatching: for a block command find the matching opening opening or 
 * closing statement
 */
Statement *Template_findMatching(Template *template, Statement *s){
	size_t currIndex;
	Statement *p = NULL;
	int depth = 0;
	StatementType type;
	
	if(s == NULL){
		/* Assume current statement */
		currIndex = List_currentIndex(template->statements);
		type      = ((Statement *)List_current(template->statements))->type;
	}
	else{
		currIndex = List_indexOf(template->statements, s);
		type      = s->type;
	}
	
	if(type == ST_BEGIN){
		/* Find matching closing statement */
		currIndex += 1;
		while(currIndex < List_length(template->statements)){
			p = (Statement *)List_get(template->statements, currIndex);
			if(p->type == ST_BEGIN) depth++;
			else if(p->type == ST_END){
				if(depth == 0) return p;
				depth--;
			}
			currIndex++;
		}
	}
	else if(type == ST_END){
		/* Find matching opening statement */
		if(currIndex != 0){
			currIndex -= 1;
			while(true){
				p = (Statement *)List_get(template->statements, currIndex);
				if(p->type == ST_END) depth++;
				else if(p->type == ST_BEGIN){
					if(depth == 0) return p;
					depth--;
				}
				if(currIndex == 0) break;
				currIndex--;
			}
		}
	}
	return s;
} /* Template_findMatching */

