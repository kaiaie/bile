/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.4 2006/03/27 23:33:28 ken Exp $
 * BileObj - The BILE object model: Publication, Section, Index, Story
 */
#ifndef BILEOBJ_H
#define BILEOBJ_H
#include "bool.h"
#include "List.h"
#include "Dict.h"
#include "Vars.h"

typedef struct _section{
	Vars       *variables;
	List       *sections;
	List       *indexes;
	List       *stories;
} Section;

/* The Publication is just the top-level Section */
typedef Section Publication;

typedef struct _story{
	Vars       *variables;
} Story;

typedef struct _index{
	char       *name;
	Vars       *variables;
	List       *stories;
} Index;


Publication *new_Publication();
void Publication_dump(Publication *p);

Section *new_Section(Section *parent);
Story   *new_Story(Section *parent);
Index   *new_Index(Section *parent, const char *name);
bool    Index_add(Index *idx, Story *st);
void Index_dump(Index *idx);

#endif /* BILEOBJ_H */
