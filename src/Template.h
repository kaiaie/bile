/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Template.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 */
#ifndef TEMPLATE_H
#define TEMPLATE_H
#include "List.h"

typedef List Template;

/* Template creation and destruction */
Template *Template_compile(char *fileName);
void     Template_execute(Template *template, char *inputFile, FILE *op);
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
