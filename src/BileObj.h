/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.8 2006/05/04 14:35:14 ken Exp $
 * BileObj - The BILE object model: Publication, Section, Index, Story
 */
#ifndef BILEOBJ_H
#define BILEOBJ_H
#include "bool.h"
#include "Dict.h"
#include "List.h"
#include "Template.h"
#include "Vars.h"

typedef enum {BILE_PUB, BILE_SECTION, BILE_STORY, BILE_INDEX} BileObjType;

typedef struct _section{
	BileObjType type;
	char        *dir;
	Vars        *variables;
	List        *sections;
	List        *indexes;
	List        *stories;
} Section;

typedef struct _publication {
	BileObjType type;
	char        *inputDirectory;
	char        *outputDirectory;
	char        *templateDirectory;
	bool        forceMode;
	bool        verboseMode;
	Dict        *templateCache;
	Section     *root;
} Publication;

typedef struct _story{
	BileObjType type;
	Section     *parent;
	Vars        *variables;
} Story;

typedef struct _index{
	BileObjType type;
	char        *name;
	Vars        *variables;
	List        *stories;
} Index;


Publication *new_Publication(char *inputDirectory, char *outputDirectory, 
	char *templateDirectory, bool forceMode, bool verboseMode);
void     Publication_build(Publication *p);
void     Publication_generate(Publication *p);
Template *Publication_getTemplate(Publication *p, char *fileName);
void     Publication_dump(Publication *p);

Section *new_Section(Section *parent, char *dir);
Story   *new_Story(Section *parent);
Index   *new_Index(Section *parent, const char *name);
bool    Index_add(Index *idx, Story *st);
void Index_dump(Index *idx);

#endif /* BILEOBJ_H */
