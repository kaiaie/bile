/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: ImgHandler.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 * ImgHandler - handler for image files 
 */
#ifndef _IMGHANDLER_H
#define _IMGHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Vars.h"
#include "FileHandler.h"

bool imgCanHandle(char *fileName);
void imgReadMetadata(char *fileName, Vars *data);
WriteStatus imgWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* _HTMLHANDLER_H */
