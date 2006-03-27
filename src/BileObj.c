/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.c,v 1.3 2006/03/27 23:33:28 ken Exp $
 */
#include <string.h>
#include <unistd.h>
#include "BileObj.h"
#include "astring.h"
#include "Logging.h"
#include "memutils.h"
#include "TextFile.h"
#include "Type.h"

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


bool Index_add(Index *idx, Story *st){
	size_t ii;
	Story *ss = NULL;
	char *sortVar = NULL;
	bool added = false;
	
	/* Skip if noindex variable is defined and true
	 * ( noindex = TRUE is a bit ass-backwards but I think it's better to index 
	 * by default)
	 */
	if(Vars_defined(st->variables, "noindex") && Type_toBool(Vars_get(st->variables, "noindex")))
		return true;
	if(List_length(idx->stories) == 0)
		List_append(idx->stories, st);
	else{
		sortVar = Vars_get(idx->variables, "sort_by");
		/* Simple insertion sort; should be OK for small indexes */
		/* Note: uses strcmp; could be a problem with numeric values */
		if(sortVar[0] == '+'){
			for(ii = 0; ii < List_length(idx->stories); ++ii){
				ss = (Story *)List_get(idx->stories, ii);
				if(strcmp(Vars_get(ss->variables, &sortVar[1]), 
					Vars_get(st->variables, &sortVar[1])) == 1){
					List_insert(idx->stories, ii, st);
					Logging_debugf("Added story %s at position %ul of index %s",
						Vars_get(st->variables, "file_name"),
						ii,
						idx->name
					);
					added = true;
					break;
				}
			}
			if(!added){
				List_append(idx->stories, st);
				Logging_debugf("Added story %s at end of index %s",
					Vars_get(st->variables, "file_name"),
					idx->name
				);
			}
		}
		else{
			for(ii = List_length(idx->stories); ii >= 1; --ii){
				ss = (Story *)List_get(idx->stories, ii - 1);
				if(strcmp(Vars_get(ss->variables, &sortVar[1]), 
					Vars_get(st->variables, &sortVar[1])) == -1){
					List_insert(idx->stories, ii, st);
					Logging_debugf("Added story %s at position %ul of index %s",
						Vars_get(st->variables, "file_name"),
						ii,
						idx->name
					);
					added = true;
					break;
				}
			}
			if(!added){
				List_insert(idx->stories, 0, st);
				Logging_debugf("Added story %s at beginning of index %s",
					Vars_get(st->variables, "file_name"),
					idx->name
				);
			}
		}
	}
	return true;
}


void Index_dump(Index *idx){
	size_t ii;
	Story *st = NULL;
	
	Logging_debugf("Index: %s", idx->name);
	Logging_debug("Index variables:");
	Vars_dump(idx->variables);
	Logging_debug("Indexed files:");
	for(ii = 0; ii < List_length(idx->stories); ++ii){
		st = (Story *)List_get(idx->stories, ii);
		Logging_debugf("\t%s", Vars_get(st->variables, "file_name"));
	}
}

