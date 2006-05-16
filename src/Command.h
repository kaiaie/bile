/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Command.h,v 1.1 2006/05/16 13:02:09 ken Exp $
 */
#ifndef COMMAND_H
#define COMMAND_H
#include "bool.h"

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


void Command_registerSimple(char *name, Action (*callback)());
void Command_registerBlock(char *name, Action (*begin)(), Action (*end)());
bool Command_exists(char *name);
Command *Command_find(char *name);


#endif /* COMMAND_H */
