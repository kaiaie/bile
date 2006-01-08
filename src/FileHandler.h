/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: FileHandler.h,v 1.2 2006/01/08 18:02:53 ken Exp $
 * FileHandler - BILE uses a Dict of FileHandler structures to process files of 
 * a particular type.  When BILE encounters a file, it calls the canHandle() 
 * function of each FileHandler and passes the filename.  The canHandle() 
 * function decides whether the particular handler can extract meaningful 
 * metadata from files of this type.  If the handler's canHandle() function 
 * returns True, BILE will call the readMetadata() function of each responding 
 * handler to build the file's metadata.  When it comes to write the output 
 * file, it calls the writeFile() function of each supporting handler until 
 * one of them returns OK or COPYORIGINAL.
 *
 * The corresponding C file contains the canHandle(), readMetadata(), and 
 * writeFile() implementations for the default file handler.  It extracts the 
 * basic metadata (e.g. file size, modification date, etc.) and implements the 
 * VERBATIM, BASE64 and SGMLENCODE output methods.
 */
#ifndef _FILEHANDLER_H
#define _FILEHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Dict.h"

/* How the file's contents should be formatted when written to the output.  
 * A handler is not required to implement all these formats; if its writeFile()
 * function returns UNSUPPORTED, BILE will try another handler until it finds 
 * one that will.
 * VERBATIM   = copy the content of the file into the template
 * BASE64     = base64-encode the content before copying it
 * SGMLENCODE = escape those characters (&, <, >, ") meaningful to SGML
 * HTMLBODY   = assume input is HTML and copy the part of the file in the 
 *              <BODY> element.
 * HTMLLINK   = Write an HTML <A> element pointing to the file.
 * HTMLINLINE = Write an HTML <IMG> or <OBJECT> element
 */
typedef enum {WF_VERBATIM, WF_BASE64, WF_SGMLENCODE, WF_HTMLBODY, WF_HTMLLINK, 
		WF_HTMLINLINE} WriteFormat;

/* Return code for the writeFile() function:
 * ERROR        = something bad happened
 * OK           = body written succesfully
 * COPYORIGINAL = success, but copy original file (for when a link to the file 
 *                rather than its actual contents is written)
 * UNSUPPORTED  = this function doesn't support the requested format; try 
 *                another handler.
 */
typedef enum {WS_ERROR = -1, WS_OK, WS_COPYORIGINAL, WS_UNSUPPORTED} 
		WriteStatus;

typedef struct _filehandler{
	bool (*canHandle)(char *fileName);
	void (*readMetadata)(char *fileName, Dict *data);
	WriteStatus (*writeFile)(char *fileName, WriteFormat format, FILE *output);
} FileHandler;



FileHandler *new_FileHandler(bool (*canHandle)(char *fileName), 
		void (*readMetadata)(char *fileName, Dict *data), 
		WriteStatus (*writeFile)(char *fileName, WriteFormat format, FILE *output));
void delete_FileHandler(FileHandler *fh);

/* Default implementations of handler functions */
bool defaultCanHandle(char *fileName);
void defaultReadMetadata(char *fileName, Dict *data);
WriteStatus defaultWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* _FILEHANDLER_H */
