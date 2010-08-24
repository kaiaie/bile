/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: ImgHandler.h,v 1.4 2010/08/24 22:10:37 ken Exp $
 */
/**
*** \file ImgHandler.h
*** \brief Handler for extracting metadata from image files (GIF, PNG, JPEG) 
**/
#ifndef IMGHANDLER_H
#define IMGHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Vars.h"
#include "FileHandler.h"

bool imgCanHandle(char *fileName);
void imgReadMetadata(char *fileName, Vars *data);
WriteStatus imgWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* IMGHANDLER_H */
