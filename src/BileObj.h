/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.19 2010/08/24 11:32:10 ken Exp $
 */
/** 
 * \file BileObj.h
 * \brief "objects" used by BILE
 *
 * The top-level BILE "object" is the \b Publication. A Publication consists 
 * of a number of \b Sections.  Each Section can contain a number of \b Stories 
 * as well as subsections.  Each Section can have a number of \b Index that sort 
 * the Stories in a particular order.
 */
#ifndef BILEOBJ_H
#define BILEOBJ_H
#include <stdio.h>
#include "Dict.h"
#include "List.h"
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

typedef struct tag_section {
	BileObjType             type;
	struct      tag_section *parent;
	char                    *dir;
	Vars                    *variables;
	List                    *sections;
	List                    *indexes;
	List                    *stories;
} Section;

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

typedef struct tag_tags {
	BileObjType type;
	char        *name;
	Vars        *variables;
	Dict        *tags;
} Tags;

typedef struct tag_story {
	BileObjType type;
	Section     *parent;
	Vars        *variables;
	char        *inputPath;
	Dict        *tags;
} Story;

typedef struct tag_index {
	BileObjType type;
	Section     *parent;
	char        *name;
	Vars        *variables;
	List        *stories;
} Index;

#endif /* BILEOBJ_H */
