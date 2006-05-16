/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Command.c,v 1.2 2006/05/16 13:30:01 ken Exp $
 */
#include "Command.h"
#include <stdio.h>
#include "astring.h"
#include "bool.h"
#include "BileObj.h"
#include "Expr.h"
#include "FileHandler.h"
#include "HtmlHandler.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "path.h"
#include "stringext.h"
#include "tokenize.h"
#include "Type.h"
#include "Vars.h"

extern Publication *thePublication;

/* -------------------------------------------------------------------
 * Local functions
 * ------------------------------------------------------------------- */
static void initialize(void);
void registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)());
/* Standard BILE commands */
Action doPrintLocation(Template *t);
Action doBreak(Template *t);
Action doComment(Template *t);
Action doEndIf(Template *t);
Action doEndIndex(Template *t);
Action doFallback(Template *t);
Action doIf(Template *t);
Action doIndex(Template *t);
Action doLetSet(Template *t);
Action doPrintPart(Template *t);
Action doPrintExpression(Template *t);
Action doPrintLiteral(Template *t);
Action doPrintSection(Template *t);

/* -------------------------------------------------------------------
 * Local variables
 * ------------------------------------------------------------------- */
static List *commandList = NULL;
static bool initialized  = false;


static void initialize(void){
   if(initialized) return;
   initialized = true;
   /* Define the basic BILE commands */
   Command_registerSimple("#", doComment);
   Command_registerSimple("%", doPrintLiteral);
   Command_registerSimple("=", doPrintExpression);
   Command_registerSimple(">", doPrintExpression);
   Command_registerSimple("BODY", doPrintPart);
   Command_registerSimple("LOCATION", doPrintLocation);
   Command_registerSimple("BREAK", doBreak);
   Command_registerSimple("BREAKIF", doBreak);
   Command_registerBlock("IF", doIf, doEndIf);
   Command_registerBlock("INDEX", doIndex, doEndIndex);
   Command_registerSimple("LET", doLetSet);
   Command_registerSimple("PREAMBLE", doPrintPart);
   Command_registerSimple("SECTIONS", doPrintSection);
   Command_registerSimple("SET", doLetSet);
} /* initialize */


void registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)()){
   Command  *newCmd  = NULL;
   
   if(!initialized) initialize();
   if(Command_exists(name))
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


Action Command_doFallback(Template *t){
	return doFallback(t);
}


bool Command_exists(char *name){
   if(!initialized) initialize();

   return (Command_find(name) != NULL);
} /* Command_exists */


Command *Command_find(char *name){
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
	return cmdFound ? theCmd : NULL;
} /* Command_find */


void Command_registerBlock(char *name, Action (*begin)(), Action (*end)()){
   registerCommand(name, true, begin, end);
} /* Command_registerBlock */


void Command_registerSimple(char *name, Action (*callback)()){
   registerCommand(name, false, callback, NULL);
} /* Command_registerSimple */


void Command_debugPrintCommands(){
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
} /* Command_debugPrintCommands */


bool printEscapedHtml(const char *s, FILE *output){
	size_t ii;
	char currChar;
	
	for(ii = 0; ii < strlen(s); ++ii){
		currChar = s[ii];
		switch(currChar){
			case '&': fputs("&amp;",  output); break;
			case '<': fputs("&lt;",   output); break;
			case '>': fputs("&gt;",   output); break;
			case '"': fputs("&quot;", output); break;
			default:  fputc(currChar, output); break;
		}
	}
	return true;
}


/* printLocationSection- recursively print the section portion of the 
 * location ("breadcrumb trail")
 */
void printLocationSection(Template *t, Section *s, const char *separator, const char *basePath){
	char *sectionPath = NULL;
	char *relativePath = NULL;
	
	if(s != thePublication->root){
		printLocationSection(t, s->parent, separator, basePath);
		sectionPath = buildPath(Vars_get(s->variables, "path"), 
			Vars_get(s->variables, "index_file"));
	}
	else
		sectionPath = astrcpy(Vars_get(s->variables, "index_file"));
	relativePath = getRelativePath(sectionPath, basePath);
	fputs("<span class=\"location_section\">", t->outputFile);
	fputs("<a href=\"", t->outputFile);
	printEscapedHtml(relativePath, t->outputFile);
	fputs("\">", t->outputFile);
	if(s == thePublication->root)
		printEscapedHtml("Home", t->outputFile);
	else
		printEscapedHtml(Vars_get(s->variables, "section_title"), t->outputFile);
	fputs("</a></span>", t->outputFile);
	fputs(separator, t->outputFile);
	mu_free(relativePath);
	mu_free(sectionPath);
} /* printLocationSection */


void printSection(Template *t, Section *s, const char *basePath){
	char *sectionPath  = NULL;
	char *relativePath = NULL;
	Section *subSection = NULL;
	size_t ii;
	
	if(s != thePublication->root)
		sectionPath = buildPath(Vars_get(s->variables, "path"), 
			Vars_get(s->variables, "index_file"));
	else
		sectionPath = astrcpy(Vars_get(s->variables, "index_file"));
	relativePath = getRelativePath(sectionPath, basePath);
	fputs("<a href=\"", t->outputFile);
	fputs(relativePath, t->outputFile);
	fputs("\">", t->outputFile);
	if(s == thePublication->root)
		printEscapedHtml("Home", t->outputFile);
	else
		printEscapedHtml(Vars_get(s->variables, "section_title"), t->outputFile);
	fputs("</a>", t->outputFile);
	fputs("<ul>\n", t->outputFile);
	for(ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if(List_length(subSection->indexes) > 0){
			fputs("<li>", t->outputFile);
			printSection(t, subSection, basePath);
			fputs("</li>\n", t->outputFile);
		}
	}
	fputs("</ul>\n", t->outputFile);
	mu_free(relativePath);
	mu_free(sectionPath);
} /* printSection */


Action doBreak(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	Action result;
	char *exprResult = NULL;

	if(strequalsi(s->cmd, "BREAK")) return ACTION_BREAK;
	exprResult = evaluateExpression(s->param, t->variables);
	if(Type_toBool(exprResult))
	  result = ACTION_BREAK;
	else
	  result = ACTION_CONTINUE;
	mu_free(exprResult);
	return result;	
} /* doBreak */


Action doComment(Template *t){
	return ACTION_CONTINUE;
} /* doComment */


Action doEndIf(Template *t){
	return ACTION_CONTINUE;
} /* doEndIf */


Action doEndIndex(Template *t){
	Index *theIndex = NULL;
	Statement *s = (Statement *)List_current(t->statements);
	Statement *beginIndex = NULL;
	BileObjType templateType = *((BileObjType *)t->context);
	
	beginIndex = Template_findMatching(t, NULL);
	theIndex = (Index *)beginIndex->userData;
	if(theIndex == NULL) return ACTION_CONTINUE;
	if(s->broken || List_atEnd(theIndex->stories)){
		/* Restore original variable scope */
		t->variables = (Vars *)s->userData;
		beginIndex->userData = NULL;
		if(templateType == BILE_INDEX)
			t->inputFile = NULL;
		else if(templateType == BILE_STORY)
			t->inputFile = ((Story *)t->context)->inputPath;
		s->broken = false;
		return ACTION_CONTINUE;
	}
	else{
		List_moveNext(theIndex->stories);
		return ACTION_REPEAT;
	}
} /* doEndIndex */


Action doFallback(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	
	if((s->param == NULL) || strempty(s->param)){
		fprintf(t->outputFile, "[[%s]]", s->cmd);
	}
	else{
		fprintf(t->outputFile, "[[%s %s]]", s->cmd, s->param);
	}
	return ACTION_CONTINUE;
} /* doFallback */


Action doIf(Template *t){
	Action result;
	char *exprResult = NULL;
	
	Statement *s = (Statement *)List_current(t->statements);
	/* FIXME: Tokenise expression once and cache in userData; not working for some reason */
	exprResult = evaluateExpression(s->param, t->variables);
	if(Type_toBool(exprResult))
	  result = ACTION_ENTER;
	else
	  result = ACTION_BREAK;
	mu_free(exprResult);
	return result;
} /* doIf */


Action doIndex(Template *t){
	Index *theIndex = NULL;
	Story *theStory = NULL;
	Statement *s = (Statement *)List_current(t->statements);
	Statement *endIndex = NULL;
	BileObjType templateType = *((BileObjType *)t->context);
	char *indexName;
	
	if(s->userData == NULL){
		if(templateType == BILE_INDEX)
			theIndex = (Index *)t->context;
		else if(templateType == BILE_STORY){
			/* Evaluate expression and find index with that name */
			indexName = evaluateExpression(s->param, t->variables);
			theIndex = Index_find(thePublication, indexName);
			if(theIndex == NULL){
				Logging_warnf("Template file \"%s\", line %d: Cannot find index \"%s\"",
					t->fileName, s->lineNo, indexName
				);
				mu_free(indexName);
				return ACTION_BREAK;
			}
			mu_free(indexName);
		}
		/* Store the index */
		s->userData = theIndex;
		/* Store the current variable scope */
		endIndex = Template_findMatching(t, NULL);
		endIndex->userData = t->variables;
		List_moveFirst(theIndex->stories);
	}
	else
		theIndex = (Index *)s->userData;
	/* Skip empty index */
	if(List_length(theIndex->stories) == 0) return ACTION_BREAK;
	theStory = (Story *)List_current(theIndex->stories);
	/* Add a variable pointing to the enclosing story's path so relative paths 
	 * can be computed.
	 */
	if(templateType == BILE_STORY){
		Vars_let(theStory->variables, "current_path", 
			astrcpy(Vars_get(((Story *)t->context)->variables, "path")));
	}
	t->variables = theStory->variables;
	t->inputFile  = theStory->inputPath;
	return ACTION_ENTER;
} /* doIndex */


Action doLetSet(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	List *tokens = tokenize(s->param);
	char *varName = NULL;
	char *exprResult = NULL;

	if(List_length(tokens) > 2 && 
		((char *)List_get(tokens, 0))[0] == '$' && 
		strequals((char *)List_get(tokens, 1), "=")){
		varName = (char *)List_get(tokens, 0);
		varName = astrcpy(&varName[1]);
		List_remove(tokens, 0, true);
		List_remove(tokens, 0, true);
		exprResult = evaluateTokens(tokens, t->variables);
		if(strequalsi(s->cmd, "LET"))
			Vars_let(t->variables, varName, exprResult);
		else
			Vars_set(t->variables, varName, exprResult);
		mu_free(varName);
	}
	else{
		Logging_warnf("Syntax error in template file \"%s\", line %d.", 
			t->fileName, s->lineNo);
	}
	delete_List(tokens, true);
	return ACTION_CONTINUE;
} /* doLet */


Action doPrintLocation(Template *t){
	Section     *sx = NULL;
	Story       *st = NULL;
	BileObjType templateType = *((BileObjType *)t->context);
	char        *separator = NULL;
	char        *basePath = NULL;
	Statement   *s = (Statement *)List_current(t->statements);
	
	separator = evaluateExpression(s->param, t->variables);
	if(templateType == BILE_STORY){
		st = (Story *)t->context;
		sx = st->parent;
	}
	else if(templateType == BILE_INDEX){
		st = NULL;
		sx = ((Index *)t->context)->parent;
	}
	basePath = Vars_get(sx->variables, "path");
	printLocationSection(t, sx, separator, basePath);
	if(templateType == BILE_STORY){
		fputs("<span class=\"location_story\">", t->outputFile);
		printEscapedHtml(Vars_get(st->variables, "title"), t->outputFile);
		fputs("</span>", t->outputFile);
	}
	mu_free(separator);
	return ACTION_CONTINUE;
} /* doPrintLocation */


Action doPrintPart(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	
	if(htmlCanHandle(t->inputFile)){
		if(strequalsi(s->cmd, "BODY")){
			htmlWriteOutput(t->inputFile, WF_HTMLBODY, t->outputFile);
		}
		else if(strequalsi(s->cmd, "PREAMBLE")){
			htmlWriteOutput(t->inputFile, WF_HTMLPREAMBLE, t->outputFile);
		}
	}
	else{
		defaultWriteOutput(t->inputFile, WF_VERBATIM, t->outputFile);
	}
	return ACTION_CONTINUE;
} /* doPrintPart */


Action doPrintExpression(Template *t){
	char *exprResult = NULL;
	Statement *s = (Statement *)List_current(t->statements);
	
	/* FIXME: Tokenise expression once and cache in userData; not working for some reason */
	exprResult = evaluateExpression(s->param, t->variables);
	if(strequals(s->cmd, ">")){
		/* Emit as-is */
		fputs(exprResult, t->outputFile);
	}
	else{
		/* Emit with special characters replaced with HTML entities */
		printEscapedHtml(exprResult, t->outputFile);
	}
	mu_free(exprResult);
	return ACTION_CONTINUE;
} /* doPrintLiteral */


Action doPrintLiteral(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	fputs(s->param, t->outputFile);
	return ACTION_CONTINUE;
} /* doPrintLiteral */


Action doPrintSection(Template *t){
	char *basePath = NULL;
	Section *parent = NULL;
	BileObjType templateType = *((BileObjType *)t->context);
	
	if(templateType == BILE_STORY)
		parent = ((Story *)t->context)->parent;
	else if(templateType == BILE_INDEX)
		parent = ((Index *)t->context)->parent;
	basePath = Vars_get(parent->variables, "path");
	printSection(t, thePublication->root, basePath);
	return ACTION_CONTINUE;
} /* doPrintSection */
