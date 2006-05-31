/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.12 2006/05/31 21:49:22 ken Exp $
 * BileObj - The BILE object model: Publication, Section, Index, Story
 */
#ifndef BILEOBJ_H
#define BILEOBJ_H
#include "bool.h"
#include "Dict.h"
#include "List.h"
#include "Template.h"
#include "Vars.h"

typedef enum {BILE_PUB, BILE_SECTION, BILE_STORY, BILE_INDEX, BILE_TAGS} BileObjType;

typedef struct _section{
	BileObjType type;
	struct _section *parent;
	char        *dir;
	Vars        *variables;
	List        *sections;
	List        *indexes;
	List        *stories;
} Section;

typedef struct _tags{
	BileObjType type;
	char        *name;
	Vars        *variables;
	Dict        *tags;
} Tags;

typedef struct _publication {
	BileObjType type;
	char        *inputDirectory;
	char        *outputDirectory;
	char        *templateDirectory;
	bool        forceMode;
	bool        verboseMode;
	Dict        *templateCache;
	Section     *root;
	List        *tagList;
} Publication;

typedef struct _story{
	BileObjType type;
	Section     *parent;
	Vars        *variables;
	char        *inputPath;
	Dict        *tags;
} Story;

typedef struct _index{
	BileObjType type;
	Section     *parent;
	char        *name;
	Vars        *variables;
	List        *stories;
} Index;


Publication *new_Publication(char *inputDirectory, char *outputDirectory, 
	char *templateDirectory, bool forceMode, bool verboseMode);
void     Publication_build(Publication *p);
void     Publication_generate(Publication *p);
Template *Publication_getTemplate(Publication *p, char *fileName);
void Publication_addToIndexes(Publication *p, Section *s, Story *st);
bool Publication_addToTags(Publication *p, Story *st);
void     Publication_dump(Publication *p);

Section *new_Section(Section *parent, char *dir);
Story   *new_Story(Section *parent);
Index   *new_Index(Section *parent, const char *name);
bool    Index_add(Index *idx, Story *st);
Index   *Index_find(Publication *p, const char *name);
Tags    *new_Tags(Publication *parent, const char *name);
bool    Tags_add(Tags *t, Story *st);
void Index_dump(Index *idx);

#endif /* BILEOBJ_H */
