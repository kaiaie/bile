/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Tags.c,v 1.1 2010/07/08 21:04:24 ken Exp $
 */
#include "Tags.h"
#include <string.h>
#include "astring.h"
#include "memutils.h"
#include "stringext.h"
#include "Type.h"

Tags *new_Tags(Publication *parent, const char *name){
	Tags *t      = mu_malloc(sizeof(Tags));
	t->type      = BILE_TAGS;
	t->name      = astrcpy(name);
	t->variables = new_Vars(parent->root->variables);
	t->tags      = new_List();
	return t;
}


/** Adds the tags in a Story to the Publication tag list */
bool Tags_add(Tags *t, Story *st){
	char *tagVar    = NULL;
	char *tagSep    = NULL;
	char *tags      = NULL;
	char **tagArray = NULL;
	char *tag = NULL;
	List *l = NULL;
	size_t ii = 0, jj;
	bool found = false;
	
	/* Check story is not excluded from tags */
	if (!(Vars_defined(st->variables, "notags") && Type_toBool(Vars_get(st->variables, "notags")))){
		/* What variable is used to hold the tags? */
		tagVar = Vars_get(t->variables, "tag_by");
		/* Separator characters */
		tagSep = Vars_get(t->variables, "tag_separator");
		if (Vars_defined(st->variables, tagVar)){
			tags = Vars_get(st->variables, tagVar);
			tagArray = astrtok(tags, tagSep);
			while ((tag = tagArray[ii]) != NULL){
				if (!strempty(tag)){
					strlower(tag);
					/* Add tags to publication's tags */
					if (Dict_exists(t->tags, tag)) {
						l = (List *)Dict_get(t->tags, tag);
					}
					else {
						l = new_List();
						Dict_putSorted(t->tags, tag, l);
					}
					List_append(l, st);
					/* Add tags to story's tags */
					if (Dict_exists(st->tags, t->name)) {
						l = (List *)Dict_get(st->tags, t->name);
					}
					else {
						l = new_List();
						Dict_put(st->tags, t->name, l);
					}
					found = false;
					if (List_length(l) > 0){
						for (jj = 0; jj < List_length(l); ++jj){
							if (strcmp(tag, (char *)List_get(l, jj)) < 0){
								List_insert(l, jj, astrcpy(tag));
								found = true;
								break;
							}
							else if (strequals(tag, (char *)List_get(l, jj))) {
								found = true;
								break;
							}
						}
					}
					if (!found) List_append(l, astrcpy(tag));
				}
				ii++;
			}
			astrtokfree(tagArray);
		}
	}
	return true;
}


