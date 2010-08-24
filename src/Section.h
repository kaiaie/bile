/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Section.h,v 1.2 2010/08/24 14:43:46 ken Exp $
 */
/** 
 * \file Section.h
 * \brief The Section corresponds to a directory 
 */
#ifndef SECTION_H
#define SECTION_H
#include "BileObj.h"

Section *new_Section(Section *parent, char *dir);
Section *Section_findById(Section *start, int sectionId);

#endif /* SECTION_H */


