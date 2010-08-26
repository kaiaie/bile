/* :tabSize=4:indentSize=4:folding=indent:
** $Id: HtmlHandler.h,v 1.5 2010/08/26 09:36:40 ken Exp $
*/
/**
*** \file HtmlHandler.h
*** \brief Handles the extraction of metadata from HTML files
***
*** The HTML handler extracts the following data:
*** <ul>
*** <li>If a TITLE element appears in the HTML document's HEAD, it is extracted 
*** into a variable called \c $title</li>
*** <li>The values of any META tags are converted into variables of the same 
*** name with any characters not allowed in variable names replaced with 
*** underscores (for example, the content of a "Content-Type" META tag will be 
*** placed in a variable called \c $content_type)</li>
*** </ul>
**/
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
