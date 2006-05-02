/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: HtmlHandler.h,v 1.4 2006/05/02 23:10:07 ken Exp $
 * HtmlHandler - handler for HTML files 
 */
#ifndef HTMLHANDLER_H
#define HTMLHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Vars.h"
#include "FileHandler.h"

bool htmlCanHandle(char *fileName);
void htmlReadMetadata(char *fileName, Vars *data);
WriteStatus htmlWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* HTMLHANDLER_H */
