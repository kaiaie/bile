/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.5 2006/04/11 23:11:23 ken Exp $
 * BileObj - The BILE object model: Publication, Section, Index, Story
 */
#ifndef BILEOBJ_H
#define BILEOBJ_H
#include "bool.h"
#include "List.h"
#include "Dict.h"
#include "Vars.h"

typedef struct _publication {
	char       *dir;
	Vars       *variables;
	List       *sections;
	List       *indexes;
	List       *stories;
	Dict       *functionTable;
	char       *inputDirectory;
	char       *outputDirectory;
	char       *templateDirectory;
} Publication;

typedef struct _section{
	char       *dir;
	Vars       *variables;
	List       *sections;
	List       *indexes;
	List       *stories;
} Section;

typedef struct _story{
	Vars       *variables;
} Story;

typedef struct _index{
	char       *name;
	Vars       *variables;
	List       *stories;
} Index;


Publication *new_Publication(char *inputDirectory, char *outputDirectory, 
	char *templateDirectory);
void Publication_build(Publication *p);
void Publication_generate(Publication *p);
void Publication_dump(Publication *p);

Section *new_Section(Section *parent, char *dir);
Story   *new_Story(Section *parent);
Index   *new_Index(Section *parent, const char *name);
bool    Index_add(Index *idx, Story *st);
void Index_dump(Index *idx);

#endif /* BILEOBJ_H */
