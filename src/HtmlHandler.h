/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: HtmlHandler.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 * HtmlHandler - handler for HTML files 
 */
#ifndef _HTMLHANDLER_H
#define _HTMLHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Vars.h"
#include "FileHandler.h"

bool htmlCanHandle(char *fileName);
void htmlReadMetadata(char *fileName, Vars *data);
WriteStatus htmlWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* _HTMLHANDLER_H */
