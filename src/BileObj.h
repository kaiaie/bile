/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.14 2006/12/13 22:57:57 ken Exp $
 */
/** 
 * \file BileObj.h
 * \brief Definitions for the "objects" used by BILE
 *
 * The top-level BILE "object" is the \b Publication. A Publication consists 
 * of a number of \b Sections.  Each Section can contain a number of \b Stories 
 * as well as subsections.  Each Section can have a number of \b Index that sort 
 * the Stories in a particular order.
 */
#ifndef BILEOBJ_H
#define BILEOBJ_H
#include "bool.h"
#include "Dict.h"
#include "List.h"
#include "Template.h"
#include "Vars.h"

/** Object types */
typedef enum {
	/** Publication */
	BILE_PUB, 
	/** Section */
	BILE_SECTION, 
	/** Story */
	BILE_STORY, 
	/** Index */
	BILE_INDEX, 
	/** Tag collection */
	BILE_TAGS
} BileObjType;

/** Section type */
typedef struct _section{
	BileObjType type;
	struct _section *parent;
	char        *dir;
	Vars        *variables;
	List        *sections;
	List        *indexes;
	List        *stories;
} Section;

/** Tag type */
typedef struct _tags{
	BileObjType type;
	char        *name;
	Vars        *variables;
	Dict        *tags;
} Tags;

/** Publication type */
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

/** Story type */
typedef struct _story{
	BileObjType type;
	Section     *parent;
	Vars        *variables;
	char        *inputPath;
	Dict        *tags;
} Story;

/** Index type */
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
Index   *Publication_findIndex(Publication *p, const char *name);
Tags    *new_Tags(Publication *parent, const char *name);
Tags    *Publication_findTags(Publication *p, const char *name);
bool    Tags_add(Tags *t, Story *st);
void Index_dump(Index *idx);

#endif /* BILEOBJ_H */
