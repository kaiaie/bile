/* :tabSize=4:indentSize=4:folding=indent:
** $Id: ConfigFile.h,v 1.2 2010/08/31 15:11:56 ken Exp $
*/
/** \file ConfigFile.h
*** Reads a Bile configuration file
**/
#ifndef CONFIGFILE_H
#define CONFIGFILE_H
#include "BileObj.h"

void readConfigFile(Publication *p, Section *s, const char *fileName);

#endif /* CONFIGFILE_H */

