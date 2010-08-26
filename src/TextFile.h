/* :tabSize=4:indentSize=4:folding=indent:
** $Id: TextFile.h,v 1.4 2010/08/26 09:35:51 ken Exp $
*/
/** \file TextFile.h
*** \brief Reads text files regardless of line ending
**/
#ifndef TEXTFILE_H
#define TEXTFILE_H
#include <stdio.h>
#include "Buffer.h"

typedef struct tag_text_file_type{
	FILE *f;
	Buffer *b;
	int state;
} TextFile;

TextFile *new_TextFile(const char *fileName);
void delete_TextFile(TextFile *t);

const char *TextFile_readLine(TextFile *t);
void        TextFile_rewind(TextFile *t);
long        TextFile_tell(TextFile *t);

#endif /* TEXTFILE_H */
