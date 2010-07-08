/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.h,v 1.16 2010/07/08 22:16:14 ken Exp $
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

typedef struct _section{
	BileObjType type;
	struct _section *parent;
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
	char        *scriptFile;
	Dict        *templateCache;
	Section     *root;
	List        *tagList;
} Publication;

typedef struct _tags{
	BileObjType type;
	char        *name;
	Vars        *variables;
	Dict        *tags;
} Tags;

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

#endif /* BILEOBJ_H */
