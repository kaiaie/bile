/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.20 2010/08/25 09:36:11 ken Exp $
 */
/** 
*** \file BileObj.h
*** \brief Defines the main data structures used by BILE
***
*** The top-level BILE "object" is the \b Publication. A Publication consists 
*** of a number of \b Sections.  Each Section can contain a number of \b Stories 
*** as well as subsections.  Each Section can have a number of \b Index that sort 
*** the Stories in a particular order.
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

/** Section data structure */
typedef struct tag_section {
	BileObjType             type;
	struct      tag_section *parent;
	char                    *dir;
	Vars                    *variables;
	List                    *sections;
	List                    *indexes;
	List                    *stories;
} Section;

/** Publication data structure */
typedef struct tag_publication {
	BileObjType type;
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
	char        *name;
	Vars        *variables;
	Dict        *tags;
} Tags;

/** Story data structure */
typedef struct tag_story {
	BileObjType type;
	Section     *parent;
	Vars        *variables;
	char        *inputPath;
	Dict        *tags;
} Story;

/** Index data structure */
typedef struct tag_index {
	BileObjType type;
	Section     *parent;
	char        *name;
	Vars        *variables;
	List        *stories;
} Index;

#endif /* BILEOBJ_H */
