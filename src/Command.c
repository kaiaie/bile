/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Command.c,v 1.19 2010/08/26 10:21:48 ken Exp $
 */
#include "Command.h"
#include <stdio.h>
#include <string.h>
#include "astring.h"
#include "bool.h"
#include "Publication.h"
#include "Section.h"
#include "Story.h"
#include "Index.h"
#include "Tags.h"
#include "Dict.h"
#include "Expr.h"
#include "FileHandler.h"
#include "HtmlHandler.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "Pair.h"
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
void registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)(), bool isDirty);
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
Action doTags(Template *t);
Action doEndTags(Template *t);
Action doInclude(Template *t);

/* -------------------------------------------------------------------
 * Local variables
 * ------------------------------------------------------------------- */
static List *commandList = NULL;
static bool initialized  = false;


static void initialize(void){
   if(initialized) return;
   initialized = true;
   /* Define the basic BILE commands */
   Command_registerSimple("#", doComment, false);
   Command_registerSimple("%", doPrintLiteral, false);
   Command_registerSimple("=", doPrintExpression, false);
   Command_registerSimple(">", doPrintExpression, false);
   Command_registerSimple("BODY", doPrintPart, false);
   Command_registerSimple("LOCATION", doPrintLocation, false);
   Command_registerSimple("BREAK", doBreak, false);
   Command_registerSimple("BREAKIF", doBreak, false);
   Command_registerBlock("IF", doIf, doEndIf, false);
   Command_registerSimple("!INCLUDE", doInclude, false);
   Command_registerBlock("INDEX", doIndex, doEndIndex, true);
   Command_registerSimple("LET", doLetSet, false);
   Command_registerSimple("PREAMBLE", doPrintPart, false);
   Command_registerSimple("SECTIONS", doPrintSection, false);
   Command_registerSimple("SET", doLetSet, false);
   Command_registerBlock("TAGS", doTags, doEndTags, true);
} /* initialize */


void registerCommand(char *name, bool isBlock, Action (*begin)(), Action (*end)(), bool isDirty){
   Command  *newCmd  = NULL;
   
   if(!initialized) initialize();
   if(Command_exists(name))
      Logging_fatalf("%s: Command \"%s\"already exists!", __FUNCTION__, name);
   newCmd = (Command *)mu_malloc(sizeof(Command));
   newCmd->name    = name;
   newCmd->isBlock = isBlock;
   newCmd->isDirty = isDirty;
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


/** Returns the Command with the specified name, or NULL if no Command with that 
*** name exists
**/
Command *Command_find(char *name){
	size_t ii;
	Command  *theCmd  = NULL;
	bool     cmdFound = false;
	
	if(!initialized) initialize();
	if(commandList != NULL){
		for(ii = 0; ii < List_length(commandList); ++ii){
			theCmd = (Command *)List_get(commandList, ii);
			if(strxequalsi(theCmd->name, name)){
				cmdFound = true;
				break;
			}
		}
	}
	return cmdFound ? theCmd : NULL;
} /* Command_find */


/**
 * \brief Registers a block command
 *
 * \param name The name of the command as it appears in a template
 * \param begin The callback function to be executed when the opening 
 * command ([[name]])is encountered
 * \param end The callback function to be executed when the closing command 
 * ([[/name]]) is encountered
 * \param isDirty If True, the presence of this command in a template 
 * means that it should always be regenerated even if the Story file hasn't 
 * changed
 * \sa Template_execute
 */
void Command_registerBlock(char *name, Action (*begin)(), Action (*end)(), bool isDirty){
   registerCommand(name, true, begin, end, isDirty);
} /* Command_registerBlock */


/**
*** \brief Registers a simple (non-block) command
*** 
*** \param name The name of the command as it appears in a template
*** \param callback The callback function to be executed when the command is 
*** encountered
*** \param isDirty If True, the presence of this command in a template 
*** means that it should always be regenerated even if the Story file hasn't 
*** changed
*** \sa Template_execute
**/
void Command_registerSimple(char *name, Action (*callback)(), bool isDirty){
   registerCommand(name, false, callback, NULL, isDirty);
} /* Command_registerSimple */


/** Writes out all defined commands (for debugging purposes only) */
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


/** Writes characters to the specified file, replacing special SGML characters 
*** with entity references
**/
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


/** Recursively prints the section portion of the location ("breadcrumb trail") 
***/
void printLocationSection(Template *t, Section *s, const char *separator, const char *basePath){
	char *sectionPath = NULL;
	char *relativePath = NULL;
	
	if (s != thePublication->root) {
		printLocationSection(t, s->parent, separator, basePath);
		sectionPath = buildPath(Vars_get(s->variables, "path"), 
			Vars_get(s->variables, "index_file"));
	}
	else {
		sectionPath = astrcpy(Vars_get(s->variables, "index_file"));
	}
	relativePath = getRelativePath(sectionPath, basePath);
	fputs("<span class=\"location_section\">", t->outputFile);
	fputs("<a href=\"", t->outputFile);
	printEscapedHtml(relativePath, t->outputFile);
	fputs("\">", t->outputFile);
	if (s == thePublication->root) {
		printEscapedHtml("Home", t->outputFile);
	}
	else {
		printEscapedHtml(Vars_get(s->variables, "section_title"), t->outputFile);
	}
	fputs("</a></span>", t->outputFile);
	fputs(separator, t->outputFile);
	mu_free(relativePath);
	mu_free(sectionPath);
} /* printLocationSection */


/** Recursively generates a bullet list of section names with links to each 
*** section's index page
*** \note Paths used internally by this function are prefixed with a "$" 
*** character. This has no significance other than to establish an anchor point 
*** when computing relative paths; any character could be used.
*** \sa doPrintSection
**/
void printSection(Template *t, Section *s, const char *basePath){
	char *sectionPath  = NULL;
	char *relativePath = NULL;
	char *tmp          = NULL;
	Section *subSection = NULL;
	size_t ii;
	
	if (s != thePublication->root) {
		tmp = buildPath(Vars_get(s->variables, "path"), 
			Vars_get(s->variables, "index_file"));
		sectionPath = astrcat("$/", tmp);
		mu_free(tmp);
	}
	else {
		if (Vars_defined(s->variables, "index_file")) {
			sectionPath = astrcat("$/", Vars_get(s->variables, "index_file"));
		}
		else {
			Logging_warn("No index file defined; assuming index.html");
			sectionPath = astrcpy("$/index.html");
		}
	}
	relativePath = getRelativePath(sectionPath, basePath);
	fprintf(t->outputFile, "<a class=\"section_list_title\" id=\"section_list_%s\" href=\"%s\">",
		Vars_get(s->variables, "section_id"), relativePath
	);
	if (s == thePublication->root) {
		printEscapedHtml("Home", t->outputFile);
	}
	else {
		printEscapedHtml(Vars_get(s->variables, "section_title"), t->outputFile);
	}
	fputs("</a>", t->outputFile);
	fputs("<ul class=\"section_list\">\n", t->outputFile);
	for (ii = 0; ii < List_length(s->sections); ++ii) {
		subSection = (Section *)List_get(s->sections, ii);
		if (List_length(subSection->indexes) > 0) {
			fputs("<li>", t->outputFile);
			printSection(t, subSection, basePath);
			fputs("</li>\n", t->outputFile);
		}
	}
	fputs("</ul>\n", t->outputFile);
	mu_free(relativePath);
	mu_free(sectionPath);
} /* printSection */


/** Implements the BREAK and BREAKIF commands */
Action doBreak(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	Action result;
	char *exprResult = NULL;

	if (strxequalsi(s->cmd, "BREAK")) return ACTION_BREAK;
	exprResult = evaluateExpression(s->param, t->variables);
	if(Type_toBool(exprResult)) {
		result = ACTION_BREAK;
	}
	else {
		result = ACTION_CONTINUE;
	}
	mu_free(exprResult);
	return result;	
} /* doBreak */


/** Implements the comment command; that is, does nothing! */
Action doComment(Template *t){
	return ACTION_CONTINUE;
} /* doComment */


/** Implements the fallback action when no command with the specified name is 
*** found
**/
Action doFallback(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	
	if((s->param == NULL) || strxempty(s->param)){
		fprintf(t->outputFile, "[[%s]]", s->cmd);
	}
	else{
		fprintf(t->outputFile, "[[%s %s]]", s->cmd, s->param);
	}
	return ACTION_CONTINUE;
} /* doFallback */


/** Implements the IF command
**/
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


Action doEndIf(Template *t){
	return ACTION_CONTINUE;
} /* doEndIf */


/** Implements the INDEX command. Switches the variable context to that of the 
*** current Story in the index.
**/
Action doIndex(Template *t){
	Index *theIndex = NULL;
	Story *theStory = NULL;
	Statement *s = (Statement *)List_current(t->statements);
	Statement *endIndex = NULL;
	BileObjType templateType = *((BileObjType *)t->context);
	char *indexName;
	
	if (s->userData == NULL){
		if(templateType == BILE_INDEX && strxnullorempty(s->param))
			/* Generating the index page for an index */
			theIndex = (Index *)t->context;
		else{
			/* Generating an index on a page */
			indexName = evaluateExpression(s->param, t->variables);
			theIndex = Publication_findIndex(thePublication, indexName);
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
		/* List_moveFirst(theIndex->stories); */
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
			Vars_get(((Story *)t->context)->variables, "path"), VAR_STD);
	}
	t->variables = theStory->variables;
	t->inputFile  = theStory->inputPath;
	return ACTION_ENTER;
} /* doIndex */


/** Implements the closing element of an INDEX block. If there are more Stories
*** in the Index being generated, the Index advances to the next Story and the 
*** block is repeated. If there are no more Stories, the Index's variable 
*** context is restored and processing of the template proceeds to the next 
*** command.
**/
Action doEndIndex(Template *t){
	Index *theIndex = NULL;
	Statement *s = (Statement *)List_current(t->statements);
	Statement *beginIndex = NULL;
	BileObjType templateType = *((BileObjType *)t->context);
	
	beginIndex = Template_findMatching(t, NULL);
	theIndex = (Index *)beginIndex->userData;
	if (theIndex == NULL) return ACTION_CONTINUE;
	if (s->broken || List_atEnd(theIndex->stories)) {
		if (s->broken) List_moveNext(theIndex->stories);
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
	else {
		List_moveNext(theIndex->stories);
		return ACTION_REPEAT;
	}
} /* doEndIndex */


Action doLetSet(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	List *tokens = tokenize(s->param);
	char *varName = NULL;
	char *exprResult = NULL;

	if (List_length(tokens) > 2 && 
		((char *)List_get(tokens, 0))[0] == '$' && 
		strxequals((char *)List_get(tokens, 1), "=")
	) {
		varName = (char *)List_get(tokens, 0);
		varName = astrcpy(&varName[1]);
		List_remove(tokens, 0, true);
		List_remove(tokens, 0, true);
		exprResult = evaluateTokens(tokens, t->variables);
		if(strxequalsi(s->cmd, "LET")) {
			Vars_let(t->variables, varName, exprResult, VAR_STD);
		}
		else {
			Vars_set(t->variables, varName, exprResult, VAR_STD);
		}
		mu_free(varName);
		mu_free(exprResult);
	}
	else {
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
	bool        canProceed = false;
	
	separator = evaluateExpression(s->param, t->variables);
	if (templateType == BILE_STORY) {
		st = (Story *)t->context;
		sx = st->parent;
		canProceed = true;
	}
	else if (templateType == BILE_INDEX) {
		st = NULL;
		sx = ((Index *)t->context)->parent;
		canProceed = true;
	}
	else {
		Logging_warnf("Unsupported!");
		canProceed = false;
	}
	if (canProceed) {
		basePath = Vars_get(sx->variables, "path");
		printLocationSection(t, sx, separator, basePath);
		if (templateType == BILE_STORY) {
			fputs("<span class=\"location_story\">", t->outputFile);
			printEscapedHtml(Vars_get(st->variables, "title"), t->outputFile);
			fputs("</span>", t->outputFile);
		}
	}
	mu_free(separator);
	return ACTION_CONTINUE;
} /* doPrintLocation */


Action doPrintPart(Template *t) {
	Statement *s = (Statement *)List_current(t->statements);
	
	if (htmlCanHandle(t->inputFile)) {
		if (strxequalsi(s->cmd, "BODY")) {
			htmlWriteOutput(t->inputFile, WF_HTMLBODY, t->outputFile);
		}
		else if (strxequalsi(s->cmd, "PREAMBLE")) {
			htmlWriteOutput(t->inputFile, WF_HTMLPREAMBLE, t->outputFile);
		}
	}
	else {
		defaultWriteOutput(t->inputFile, WF_VERBATIM, t->outputFile);
	}
	return ACTION_CONTINUE;
} /* doPrintPart */


/** Evaluates an expression and writes the result to the template output file */
Action doPrintExpression(Template *t){
	char *exprResult = NULL;
	Statement *s = (Statement *)List_current(t->statements);
	
	/* FIXME: Tokenise expression once and cache in userData; not working for some reason */
	exprResult = evaluateExpression(s->param, t->variables);
	if (strxequals(s->cmd, ">")) {
		/* Emit as-is */
		fputs(exprResult, t->outputFile);
	}
	else {
		/* Emit with special characters replaced with HTML entities */
		printEscapedHtml(exprResult, t->outputFile);
	}
	mu_free(exprResult);
	return ACTION_CONTINUE;
} /* doPrintExpression */


/** Writes a literal string to the template's output file */
Action doPrintLiteral(Template *t){
	Statement *s = (Statement *)List_current(t->statements);
	fputs(s->param, t->outputFile);
	return ACTION_CONTINUE;
} /* doPrintLiteral */


/** Generates a nested bullet list of Section names and links to each Section's 
*** index page.
*** \sa printSection
**/
Action doPrintSection(Template *t){
	char        *basePath       = NULL;
	char        *tmp            = NULL;
	char        *exprResult     = NULL;
	Section     *parent         = NULL;
	Section     *start          = thePublication->root;
	Statement   *stmt           = (Statement *)List_current(t->statements);
	BileObjType templateType    = *((BileObjType *)t->context);
	
	if (templateType == BILE_STORY) {
		tmp =  adirname(Vars_get(((Story *)t->context)->variables, "path"));	
		basePath = astrcat("$/", tmp);
		mu_free(tmp);
	}
	else if (templateType == BILE_INDEX) {
		parent = ((Index *)t->context)->parent;
		/* On an index page, the SECTIONS command can take an optional boolean 
		** parameter indicating whether all the sections should be listed or 
		** just the subsections of the current section.  This parameter is 
		** ignored elsewhere.
		*/
		if (!strxnullorempty(stmt->param)) {
			exprResult = evaluateExpression(stmt->param, t->variables);
			if (!Type_toBool(exprResult)) {
				start = parent;
			}
			mu_free(exprResult);
		}
		if (parent == thePublication->root) {
			basePath = astrcpy("$/");
		}
		else {
			basePath = astrcat("$/", Vars_get(((Index *)t->context)->variables, "path"));
		}
	}
	else if(templateType == BILE_TAGS) {
		basePath = astrcpy("$/");
	}
	printSection(t, start, basePath);
	mu_free(basePath);
	return ACTION_CONTINUE;
} /* doPrintSection */


/** Generates a list of Tags in a Story, or an index page of Tags */
Action doTags(Template *t){
	Statement   *s           = (Statement *)List_current(t->statements);
	BileObjType templateType = *((BileObjType *)t->context);
	Story       *st          = NULL;
	char        *tagListName = NULL;
	char        *tagFileExt  = NULL;
	char        *tagFileName = NULL;
	char        *basePath    = NULL;
	Tags        *theTags     = NULL;
	Pair        *p           = NULL;
	
	if (templateType == BILE_TAGS) {
		/* Generating a tag page */
		theTags = (Tags *)t->context;
		/* Skip if no tags have been defined */
		if(List_length((List *)theTags->tags) == 0) return ACTION_CONTINUE;
		p = (Pair *)List_current((List *)theTags->tags);
		/* Skip if no stories have this particular tag */
		if(List_length((List *)p->value) == 0) return ACTION_CONTINUE;
		if(s->userData == NULL){
			/* Save variables */
			s->userData = t->variables;
		}
		/* The tags field of the Tags structure is a Dict whose key is the 
		 * tag and whose value is a List of all Stories that contain the tag
		 * (yes, the word "tag" is gratuitously overused!)
		 */
		st = (Story *)List_current((List *)p->value);
		Vars_set(st->variables, "current_tag", p->key, VAR_STD);
		/* Use the variables of the current story while in this block */
		t->variables = st->variables;
	}
	else if (templateType == BILE_STORY) {
		/* List the tags for this story in the tag list */
		st = (Story *)t->context;
		if(s->userData == NULL){
			/* First time */
			tagListName = evaluateExpression(s->param, st->variables);
			if(Dict_exists(st->tags, tagListName)) {
				s->userData = tagListName;
				List_moveFirst((List *)Dict_get(st->tags, tagListName));
			}
			else {
				mu_free(tagListName);
				return ACTION_CONTINUE;
			}
		}
		/* Add tag variables */
		/* The tag itself */
		Vars_set(st->variables, 
			"current_tag", List_current((List *)Dict_get(st->tags, s->userData)), VAR_STD);
		/* The name of the tag file to point to */
		basePath = Vars_get(st->variables, "path");
		theTags = Publication_findTags(thePublication, s->userData);
		if (Vars_defined(theTags->variables, "tag_file")) {
			/* Single file mode */
			tagFileName = astrcpy(Vars_get(theTags->variables, "tag_file"));
		}
		else {
			/* Multi-file mode */
			tagFileExt  = getPathPart(Vars_get(theTags->variables, "tag_template"), PATH_EXT);
			tagFileName = asprintf("tag_%s_%s.%s", theTags->name, 
				Vars_get(st->variables, "current_tag"),
				tagFileExt
			);
			mu_free(tagFileExt);
		}
		Vars_set(st->variables, "current_tag_file", tagFileName, VAR_STD);
		mu_free(tagFileName);
	}
	else {
		Logging_warn("Cannot use the TAGS command here.");
		return ACTION_CONTINUE;
	}
	return ACTION_ENTER;
} /* doTags */


Action doEndTags(Template *t) {
	Statement   *currStmt  = (Statement *)List_current(t->statements);
	Statement   *beginStmt = Template_findMatching(t, currStmt);
	BileObjType templateType = *((BileObjType *)t->context);
	Story       *theStory  = NULL;
	Tags        *theTags   = NULL;
	List        *storyList = NULL;
	List        *tagList   = NULL;
	Pair        *p         = NULL;
	Action      result     = ACTION_CONTINUE;
	
	if (templateType == BILE_STORY) {
		theStory = (Story *)t->context;
		tagList = (List *)Dict_get(theStory->tags, (char *)beginStmt->userData);
		if (tagList != NULL && List_moveNext(tagList)) {
			result = ACTION_REPEAT;
		}
		else {
			mu_free(beginStmt->userData);
			beginStmt->userData = NULL;
			result = ACTION_CONTINUE;
		}
	}
	else if (templateType == BILE_TAGS) {
		theTags   = (Tags *)t->context;
		if (List_length((List *)theTags->tags) == 0) {
			result = ACTION_CONTINUE;
		}
		else {
			p = (Pair *)List_current((List *)theTags->tags);
			storyList = (List *)p->value;
			if (List_length(storyList) == 0) {
				result = ACTION_CONTINUE;
			}
			else {
				if (currStmt->broken) {
					/* Restore variables */
					t->variables = beginStmt->userData;
					result = ACTION_CONTINUE;
				}
				else if (!List_moveNext(storyList)) {
					/* Move to next tag, if one exists */
					/* Continue if in multi-file mode, otherwise loop */
					if (!Vars_defined(theTags->variables, "tag_file")) {
						/* Restore variables */
						t->variables = beginStmt->userData;
						result = ACTION_CONTINUE;
					}
					else if (!List_moveNext((List *)theTags->tags)) {
						result = ACTION_CONTINUE;
					}
					else {
						result = ACTION_REPEAT;
					}
				}
				else {
					result = ACTION_REPEAT;
				}
			}
		}
	}
	else {
		result = ACTION_CONTINUE;
	}
	return result;
} /* doEndTags */


/** Implements the INCLUDE command 
*** \note INCLUDE is an immediate command; it is executed at Template compile 
*** time, not execution time
**/
Action doInclude(Template *t){
	char     **cmdData   = (char **)t->context;
	char     *cmd  __attribute__ ((unused)) = cmdData[0];
	char     *args       = cmdData[1];
	Template *sub        = NULL;
	char     *exprResult = NULL;
	size_t   ii;
	
	exprResult = evaluateExpression(args, thePublication->root->variables);
	if (fileExists(exprResult)) {
		/* Compile sub-template and copy its Statements into the parent */
		sub = Template_compile(exprResult);
		for(ii = 0; ii < List_length(sub->statements); ++ii)
			List_append(t->statements, List_get(sub->statements, ii));
		/* Update the timestamp */
		if(sub->timestamp > t->timestamp) t->timestamp = sub->timestamp;
		mu_free(sub->fileName);
		mu_free(sub);
	}
	else {
		Logging_warnf("Cannot find included template file \"%s\"", exprResult);
	}
	mu_free(exprResult);
	return ACTION_CONTINUE;
} /* doInclude */

