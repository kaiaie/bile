/** 
*** \file Section.h
*** \brief The Section corresponds to a directory 
**/
#ifndef SECTION_H
#define SECTION_H
#include "BileObj.h"

Section *new_Section(Section *parent, char *dir);
Section *Section_findById(Section *start, int sectionId);

#endif /* SECTION_H */


