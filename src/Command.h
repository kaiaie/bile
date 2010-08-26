/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Command.h,v 1.6 2010/08/26 10:19:19 ken Exp $
**/
/** \file Command.h
*** \brief Implements the commands used by the template processor
***
*** A Command can be a <em>simple command</em>, or a <em>block command</em> 
*** which can contain other Commands
***
*** \sa Template
**/
#ifndef COMMAND_H
#define COMMAND_H
#include "bool.h"
#include "Template.h"

/** \brief Status codes returned by callback functions */
typedef enum {
	/** \brief Exit template execution  */
	ACTION_ABORT = -1,
	/** \brief Go to next command */
	ACTION_CONTINUE,
	/** \brief Enter block */
	ACTION_ENTER,
	/** \brief Return to start of block */
	ACTION_REPEAT,
	/** \brief Exit enclosing block */
	ACTION_BREAK
} Action;

/** \brief Structure describing each command */
typedef struct tag_command {
	/** \brief The name of the command as it appears in a template file */
	char   *name;
	/** \brief True if the command is a block command, False otherwise */
	bool   isBlock;
	/** \brief If true, then any template with this command in it 
	*** should always be regenerated.
	**/
	bool   isDirty;
	/** \brief Pointer to a callback function that is called when the command is 
	***encountered
	**/
	Action (*begin)();
	/** \brief Pointer to a callback function that is called when the closing 
	*** block of the command is encountered (block commands only)
	**/
	Action (*end)();
} Command;


Action Command_doFallback(Template *t);
bool Command_exists(char *name);
Command *Command_find(char *name);
void Command_registerBlock(char *name, Action (*begin)(), Action (*end)(), bool isDirty);
void Command_registerSimple(char *name, Action (*callback)(), bool isDirty);

/* Debugging functions */
void Command_debugPrintCommands(void);

#endif /* COMMAND_H */
