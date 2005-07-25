#ifndef _TEMPLATE_H
#define _TEMPLATE_H
#include "DList.h"

typedef DList Template;

/* Template creation and destruction */
Template *Template_compile(char *fileName);
void     Template_execute(Template *template, FILE *op);
void     delete_Template(Template *t);


/* Debugging functions */
void Bile_debugPrintCommands(void);
void Template_debugPrintTemplate(Template *template);


/* Status codes returned by callback functions */
typedef enum {ACTION_ABORT = -1, ACTION_CONTINUE, ACTION_ENTER, ACTION_REPEAT, ACTION_BREAK} Action;


/* Functions to add commands to the BILE template parser */
void Bile_registerCommand (char *name, Action (*callback)());
void Bile_registerBlock(char *name, Action (*begin)(), Action (*end)());

#endif /* _TEMPLATE_H */
