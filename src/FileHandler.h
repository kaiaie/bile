/* :tabSize=4:indentSize=4:folding=indent:
** $Id: FileHandler.h,v 1.5 2010/08/24 21:28:16 ken Exp $
*/
/**
*** \file FileHandler.h
*** \brief Generic file handler
***
*** BILE uses a Dict of FileHandler structures to process files of 
*** a particular type.  When BILE encounters a file, it calls the canHandle() 
*** function of each FileHandler and passes the filename.  The canHandle() 
*** function decides whether the particular handler can extract meaningful 
*** metadata from files of this type.  If the handler's canHandle() function 
*** returns True, BILE will call the readMetadata() function of each responding 
*** handler to build the file's metadata.  When it comes to write the output 
*** file, it calls the writeFile() function of each supporting handler until 
*** one of them returns OK or COPYORIGINAL.
***
*** The corresponding C file contains the canHandle(), readMetadata(), and 
*** writeFile() implementations for the default file handler.  It extracts the 
*** basic metadata (e.g. file size, modification date, etc.) and implements the 
*** VERBATIM, BASE64 and SGMLENCODE output methods.
**/
#ifndef FILEHANDLER_H
#define FILEHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Vars.h"

/** \brief How the file's contents should be formatted when written to the 
*** output.
***
*** A handler is not required to implement all these formats; if its writeFile()
*** function returns UNSUPPORTED, BILE will try another handler until it finds 
*** one that will.
**/
typedef enum {
	/** Copy the content of the file into the template without changes */
	WF_VERBATIM, 
	/** base64-encode the content before copying it */
	WF_BASE64,
	/** Escape those characters (\&, \<, \>, \") meaningful to SGML */
	WF_SGMLENCODE, 
	/** Assume input is HTML and copy the content prior to the HTML
	*** tag (for e.g. PHP or ASP code that must run before anything
	*** is emitted)
	**/
	WF_HTMLPREAMBLE,
	/** assume input is HTML and copy the part of the file in the 
	*** \<BODY\> element
	**/
	WF_HTMLBODY,
	/** Write an HTML \<A\> element pointing to the file */
	WF_HTMLLINK, 
	/** Write an HTML \<IMG\> or \<OBJECT\> element */
	WF_HTMLINLINE
} WriteFormat;

/** Return code for the writeFile() function: */
typedef enum {
	/** Something bad happened */
	WS_ERROR = -1, 
	/** Body written successfully */
	WS_OK,
	/** Success, but copy original file (for when a link to the file rather 
	*** than its actual contents is written)
	**/
	WS_COPYORIGINAL,
	/** This function doesn't support the requested format; try another handler
	**/
	WS_UNSUPPORTED
} WriteStatus;

typedef struct tag_filehandler{
	bool (*canHandle)(char *fileName);
	void (*readMetadata)(char *fileName, Vars *vars);
	WriteStatus (*writeFile)(char *fileName, WriteFormat format, FILE *output);
} FileHandler;

FileHandler *new_FileHandler(
	bool (*canHandle)(char *fileName), 
	void (*readMetadata)(char *fileName, Vars *data), 
	WriteStatus (*writeFile)(char *fileName, WriteFormat format, FILE *output)
);

void delete_FileHandler(FileHandler *fh);

/* Default implementations of handler functions */
bool defaultCanHandle(char *fileName);
void defaultReadMetadata(char *fileName, Vars *data);
WriteStatus defaultWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* FILEHANDLER_H */
