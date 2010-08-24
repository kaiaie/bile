/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Index.c,v 1.2 2010/08/24 08:37:01 ken Exp $
 */
#include "Index.h"
#include <string.h>
#include "astring.h"
#include "Logging.h"
#include "memutils.h"
#include "Type.h"
#include "Vars.h"

Index *new_Index(Section *parent, const char *name){
	Index *i = NULL;
	i = (Index *)mu_malloc(sizeof(Index));
	i->type   = BILE_INDEX;
	i->parent = parent;
	i->name   = astrcpy(name);
	i->variables = new_Vars(parent->variables);
	i->stories = new_List();
	return i;
}


bool Index_add(Index *idx, Story *st){
	size_t ii;
	Story *ss = NULL;
	char *sortVar = NULL;
	bool added = false;
	int  cmpVal = 1;
	char *indexCount = NULL;
	
	/* Skip if noindex variable is defined and true
	 * ( noindex = TRUE is a bit ass-backwards but I think it's better to index 
	 * by default)
	 */
	if (Vars_defined(st->variables, "noindex") && Type_toBool(Vars_get(st->variables, "noindex"))) {
		return true;
	}
	if (List_length(idx->stories) == 0) {
		List_append(idx->stories, st);
	}
	else {
		sortVar = Vars_get(idx->variables, "sort_by");
		/* Simple insertion sort; should be OK for small indexes */
		/* Note: uses strcmp; could be a problem with numeric values */
		cmpVal = (sortVar[0] == '+') ? 1 : -1;
		for (ii = 0; ii < List_length(idx->stories); ++ii){
			ss = (Story *)List_get(idx->stories, ii);
			if (strcmp(Vars_get(ss->variables, &sortVar[1]), 
				Vars_get(st->variables, &sortVar[1])) == cmpVal){
				List_insert(idx->stories, ii, st);
				Logging_debugf("Added story %s at position %u of index %s",
					Vars_get(st->variables, "file_name"),
					ii,
					idx->name
				);
				added = true;
				break;
			}
		}
		if (!added){
			List_append(idx->stories, st);
			Logging_debugf("Added story %s at end of index %s",
				Vars_get(st->variables, "file_name"),
				idx->name
			);
		}
		indexCount = asprintf("%d", List_length(idx->stories));
		Vars_let(idx->variables, "index_count", indexCount, VAR_STD);
		mu_free(indexCount);
	}
	return true;
}


void Index_dump(Index *idx){
	size_t ii;
	Story *st = NULL;
	
	Logging_debugf("Dumping index: %s", idx->name);
	Logging_debug("Index variables:");
	Vars_dump(idx->variables);
	Logging_debug("Indexed files:");
	for(ii = 0; ii < List_length(idx->stories); ++ii){
		st = (Story *)List_get(idx->stories, ii);
		Logging_debugf("\t%s", Vars_get(st->variables, "file_name"));
	}
}


