/** 
*** \file BileObj.h
*** \brief Defines the main data structures used by BILE
***
*** The top-level BILE "object" is the \b Publication. A Publication consists 
*** of a number of \b Sections.  Each Section can contain a number of \b Stories 
*** as well as subsections, and each Section can have a number of \b Indexes 
*** that can be used to generate index pages of Stories sorted in a particular 
*** order (for example, a blog Publication might sort the Stories in order of 
*** decreasing date, or a glossary Publication in alphabetical order) An Index 
*** defined at the Publication level includes all Stories in the Publication, 
*** whereas an Index defined in a Section only indexes the Stories in that 
*** Section, and not any of its subsections.
*** 
*** In addition to Indexes, a Publication can have one or more sets of \b Tags 
*** that allow Stories to be grouped according in one or more general 
*** categories.
**/
#ifndef BILEOBJ_H
#define BILEOBJ_H
#include <stdio.h>
#include "Dict.h"
#include "List.h"
#include "Vars.h"

/** Identifies the type of a particular BILE data structure */
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

/** Generic object type */
typedef struct tag_object {
	BileObjType type;
	Vars        *variables;
} BileObject;

/** Section data structure */
typedef struct tag_section {
	BileObjType             type;
	Vars                    *variables;
	struct      tag_section *parent;
	char                    *dir;
	List                    *sections;
	List                    *indexes;
	List                    *stories;
} Section;

/** Publication data structure */
typedef struct tag_publication {
	BileObjType type;
	Vars        *variables;
	char        *inputDirectory;
	char        *outputDirectory;
	char        *templateDirectory;
	bool        forceMode;
	bool        verboseMode;
	char        *scriptFileName;
	FILE        *scriptFile;
	Dict        *templateCache;
	Section     *root;
	List        *tagList;
} Publication;

/** Tags data structure */
typedef struct tag_tags {
	BileObjType type;
	Vars        *variables;
	char        *name;
	Dict        *tags;
} Tags;

/** Story data structure */
typedef struct tag_story {
	BileObjType type;
	Vars        *variables;
	Section     *parent;
	char        *inputPath;
	Dict        *tags;
} Story;

/** Index data structure */
typedef struct tag_index {
	BileObjType type;
	Vars        *variables;
	Section     *parent;
	char        *name;
	List        *stories;
} Index;

#endif /* BILEOBJ_H */
