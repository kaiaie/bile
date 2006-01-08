/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: TextFile.h,v 1.2 2006/01/08 18:02:54 ken Exp $
 * TextFile - A simple library for reading text files one line at a time
 */
#ifndef _TEXTFILE_H
#define _TEXTFILE_H
#include <stdio.h>
#include "Buffer.h"

typedef struct _text_file_type{
	FILE *f;
	Buffer *b;
	int state;
} TextFile;

TextFile *new_TextFile(const char *fileName);
void delete_TextFile(TextFile *t);

const char *TextFile_readLine(TextFile *t);
void        TextFile_rewind(TextFile *t);
long        TextFile_tell(TextFile *t);

#endif /* _TEXTFILE_H */
