/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Publication.h,v 1.3 2010/08/25 09:36:11 ken Exp $
 */
/** 
*** \file Publication.h
*** \brief Implements the Publication, the top-level BILE data type 
**/
#ifndef PUBLICATION_H
#define PUBLICATION_H
#include "bool.h"
#include "BileObj.h"
#include "Template.h"

Publication *new_Publication(char *inputDirectory, char *outputDirectory, 
	char *templateDirectory, bool forceMode, bool verboseMode, char *scriptFile);
void     Publication_build(Publication *p);
void     Publication_generate(Publication *p);
Template *Publication_getTemplate(Publication *p, char *fileName);
void     Publication_addToIndexes(Publication *p, Section *s, Story *st);
bool     Publication_addToTags(Publication *p, Story *st);
void     Publication_dump(Publication *p);
Index   *Publication_findIndex(Publication *p, const char *name);
Tags    *Publication_findTags(Publication *p, const char *name);

#endif /* PUBLICATION_H */

