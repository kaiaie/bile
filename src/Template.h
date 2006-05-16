/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Template.h,v 1.10 2006/05/16 13:30:01 ken Exp $
 */
#ifndef TEMPLATE_H
#define TEMPLATE_H
#include <stdio.h>
#include <time.h>
#include "List.h"
#include "Vars.h"


typedef enum {ST_SIMPLE, ST_BEGIN, ST_END} StatementType;

typedef struct _statement{
   StatementType type;
   size_t        lineNo;
   char          *cmd;
   char          *param;
   bool          broken;
   void          *userData;
} Statement;

typedef struct _template{
	char   *fileName;
	time_t timestamp;
	void   *context;
	Vars   *variables;
	char   *inputFile;
	FILE   *outputFile;
	char   *outputFileName;
	List   *statements;
}Template;

Template *new_Template(void);
Template *Template_compile(char *fileName);
void     Template_execute(Template *template, void *context, char *outputFileName);
void     delete_Template(Template *t);

Statement *Template_findMatching(Template *template, Statement *s);


/* Debugging functions */
void Template_debugPrintTemplate(Template *template);


#endif /* TEMPLATE_H */
