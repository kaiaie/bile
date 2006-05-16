/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Command.h,v 1.2 2006/05/16 13:30:01 ken Exp $
 */
#ifndef COMMAND_H
#define COMMAND_H
#include "bool.h"
#include "Template.h"

/* Status codes returned by callback functions */
typedef enum {ACTION_ABORT = -1, /* Exit template execution  */
		ACTION_CONTINUE,         /* Go to next command       */
		ACTION_ENTER,            /* Enter block              */
		ACTION_REPEAT,           /* Return to start of block */
		ACTION_BREAK             /* Exit block */
} Action;

typedef struct _command{
   char   *name;
   bool   isBlock;
   Action (*begin)();
   Action (*end)();
} Command;


Action Command_doFallback(Template *t);
bool Command_exists(char *name);
Command *Command_find(char *name);
void Command_registerBlock(char *name, Action (*begin)(), Action (*end)());
void Command_registerSimple(char *name, Action (*callback)());

/* Debugging functions */
void Command_debugPrintCommands(void);

#endif /* COMMAND_H */
