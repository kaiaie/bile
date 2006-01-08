/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.2 2006/01/08 18:02:53 ken Exp $
 * BileObj - The BILE object model: Publication, Section, Index, Story
 */
#ifndef _BILEOBJ_H
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


Publication *new_Publication(char *configFileName);

void Publication_dump(Publication *p);

#define _BILEOBJ_H
#endif /* _BILEOBJ_H */
