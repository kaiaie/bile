/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Story.c,v 1.2 2010/08/31 15:11:57 ken Exp $
*/
#include "Story.h"
#include "memutils.h"

/** Creates and allocates a new Story with the specified parent */
Story *new_Story(Section *parent) {
	Story *s = NULL;
	s = (Story *)mu_malloc(sizeof(Story));
	s->type = BILE_STORY;
	s->variables = new_Vars(parent->variables);
	s->parent = parent;
	s->inputPath = NULL;
	s->tags = new_Dict();
	/* Add default variables */
	Vars_let(s->variables, "is_new", "false", VAR_STD);
	Vars_let(s->variables, "is_modified", "false", VAR_STD);
	return s;
}


