/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Section.c,v 1.1 2010/07/08 21:04:24 ken Exp $
 */
#include "Section.h"
#include "astring.h"
#include "memutils.h"

Section *new_Section(Section *parent, char *dir){
	Section *s = NULL;
	s = (Section *)mu_malloc(sizeof(Section));
	s->type = BILE_SECTION;
	s->parent = parent;
	if(parent == NULL)
		s->variables = new_Vars(NULL);
	else
		s->variables = new_Vars(parent->variables);
	s->dir = astrcpy(dir);
	s->sections  = new_List();
	s->indexes   = new_List();
	s->stories   = new_List();
	return s;
}



