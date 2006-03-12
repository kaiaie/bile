/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.c,v 1.2 2006/03/12 01:08:03 ken Exp $
 */
#include <string.h>
#include <unistd.h>
#include "BileObj.h"
#include "astring.h"
#include "Logging.h"
#include "memutils.h"
#include "TextFile.h"

Publication *new_Publication(){
	return (Publication *)new_Section(NULL);
}

void Publication_dump(Publication *p){
	if(p != NULL){
		Logging_warnf("%s: Not implemented yet", __FUNCTION__);
	}
	else
		Logging_warnf("%s: NULL argument", __FUNCTION__);
}


Section *new_Section(Section *parent){
	Section *s = NULL;
	s = (Section *)mu_malloc(sizeof(Section));
	if(parent == NULL)
		s->variables = new_Vars(NULL);
	else
		s->variables = new_Vars(parent->variables);
	s->sections  = new_List();
	s->indexes   = new_List();
	s->stories   = new_List();
	return s;
}


Story *new_Story(Section *parent){
	Story *s = NULL;
	s = (Story *)mu_malloc(sizeof(Story));
	s->variables = new_Vars(parent->variables);
	return s;
}


Index *new_Index(Section *parent, const char *name){
	Index *i = NULL;
	i = (Index *)mu_malloc(sizeof(Index));
	i->name = astrcpy(name);
	i->variables = new_Vars(parent->variables);
	i->stories = new_List();
	return i;
}
