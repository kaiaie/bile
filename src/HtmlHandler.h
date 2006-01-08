/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: HtmlHandler.h,v 1.2 2006/01/08 18:02:54 ken Exp $
 * HtmlHandler - handler for HTML files 
 */
#ifndef _HTMLHANDLER_H
#define _HTMLHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Dict.h"
#include "FileHandler.h"

bool htmlCanHandle(char *fileName);
void htmlReadMetadata(char *fileName, Dict *data);
WriteStatus htmlWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* _HTMLHANDLER_H */
