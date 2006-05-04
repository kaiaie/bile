/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Template.h,v 1.7 2006/05/04 14:35:14 ken Exp $
 */
#ifndef TEMPLATE_H
#define TEMPLATE_H
#include <stddef.h>
#include <time.h>
#include "List.h"
#include "Vars.h"

typedef struct _template{
	char   *fileName;
	time_t timestamp;
	void   *context;
	Vars   *variables;
	char   *inputFile;
	FILE   *outputFile;
	List   *statements;
}Template;

/* Template creation and destruction */
Template *new_Template(void);
Template *Template_compile(char *fileName);
void     Template_execute(Template *template, void *context, char *inputFile, FILE *outputFile);
void     delete_Template(Template *t);


/* Debugging functions */
void Bile_debugPrintCommands(void);
void Template_debugPrintTemplate(Template *template);


/* Status codes returned by callback functions */
typedef enum {ACTION_ABORT = -1, /* Exit template execution  */
		ACTION_CONTINUE,         /* Go to next command       */
		ACTION_ENTER,            /* Enter block              */
		ACTION_REPEAT,           /* Return to start of block */
		ACTION_BREAK             /* Exit block */
} Action;


/* Functions to add commands to the BILE template parser */
void Bile_registerCommand(char *name, Action (*callback)());
void Bile_registerBlock(char *name, Action (*begin)(), Action (*end)());

#endif /* TEMPLATE_H */
