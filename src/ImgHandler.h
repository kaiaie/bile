/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: ImgHandler.h,v 1.2 2006/01/08 18:02:54 ken Exp $
 * ImgHandler - handler for image files 
 */
#ifndef _IMGHANDLER_H
#define _IMGHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Dict.h"
#include "FileHandler.h"

bool imgCanHandle(char *fileName);
void imgReadMetadata(char *fileName, Dict *data);
WriteStatus imgWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* _HTMLHANDLER_H */
