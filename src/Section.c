/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Section.c,v 1.2 2010/08/24 14:43:46 ken Exp $
 */
#include <stdlib.h>
#include "Section.h"
#include "Type.h"
#include "astring.h"
#include "memutils.h"

/** Creates a empty section */
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

/** Returns the Section with the specified ID */
Section *Section_findById(Section *start, int sectionId) {
	Section *result = NULL;
	char    *sectionIdValue = NULL;
	int     currentSectionId;
	int     ii;

	// Check if current section is the one
	if (Vars_defined(start->variables, "section_id")) {
		sectionIdValue = Vars_get(start->variables, "section_id");
		if (Type_isNumeric(sectionIdValue)) {
			currentSectionId = atoi(sectionIdValue);
			if (currentSectionId == sectionId) {
				result = start;
			}
		}
	}
	if (result == NULL) {
		// Check subsections
		for (ii = 0; ii < List_length(start->sections); ++ii) {
			result = Section_findById(
				(Section *)List_get(start->sections, ii), 
				sectionId
			);
			if (result != NULL) {
				break;
			}
		}
	}
	return result;
}



