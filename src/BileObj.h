/* :tabSize=4:indentSize=4:folding=indent:
 * BileObj - The BILE object model: Publication, Section, Index, Story
 */
#ifndef _BILEOBJ_H
#include "bool.h"
#include "List.h"
#include "Dict.h"
#include "Vars.h"

typedef struct _section{
	Dict       *variables;
	List       *sections;
	List       *indexes;
	List       *stories;
} Section;

/* The Publication is just the top-level Section */
typedef Section Publication;

typedef struct _story{
	Dict       *variables;
} Story;

typedef struct _index{
	char       *name;
	Dict       *variables;
	List       *stories;
} Index;


Publication *new_Publication(char *configFileName);

void Publication_dump(Publication *p);

#define _BILEOBJ_H
#endif /* _BILEOBJ_H */
