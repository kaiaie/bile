/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: FileHandler.c,v 1.8 2006/06/10 20:23:42 ken Exp $
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "FileHandler.h"
#include "astring.h"
#include "Logging.h"
#include "memutils.h"
#include "path.h"

FileHandler *new_FileHandler(bool (*canHandle)(char *fileName), 
		void (*readMetadata)(char *fileName, Vars *vars), 
		WriteStatus (*writeFile)(char *fileName, WriteFormat format, FILE *output)){
	FileHandler *result = NULL;
	
	result = (FileHandler *)mu_malloc(sizeof(FileHandler));
	result->canHandle    = canHandle;
	result->readMetadata = readMetadata;
	result->writeFile    = writeFile;
	return result;
}


void delete_FileHandler(FileHandler *fh){
	if(fh != NULL){
		mu_free(fh);
	}
	else{
		Logging_warnf("%s(): NULL argument", __FUNCTION__);
	}
}


bool defaultCanHandle(char *fileName){
	return true;
}


void defaultReadMetadata(char *fileName, Vars *vars){
	struct stat st;
	char *tmpFileName = NULL;
	char *tmpFileSize = NULL;
	char *tmpFileDate = NULL;
	
	tmpFileName = getPathPart(fileName, PATH_FILE);
	Vars_let(vars, "file_name", tmpFileName, VAR_STD);
	if(stat(fileName, &st) != -1){
		tmpFileSize = asprintf("%d", st.st_size);
		tmpFileDate = asprintf("%d", st.st_mtime);
		Vars_let(vars, "file_size", tmpFileSize, VAR_STD);
		Vars_let(vars, "file_date", tmpFileDate, VAR_STD);
		mu_free(tmpFileDate);
		mu_free(tmpFileSize);
	}
	else{
		Logging_warnf("%s: Unable to stat() file \"%s\": %s", 
				__FUNCTION__, fileName, strerror(errno));
	}
	mu_free(tmpFileName);
}


WriteStatus defaultWriteOutput(char *fileName, WriteFormat format, FILE *output){
	char base64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	unsigned char base64Buffer[3];
	size_t bytesRead  = 0;
	size_t lineLength = 0;
	size_t idx;
	WriteStatus result  = WS_OK;
	FILE        *in     = NULL;
	int currChr;
	
	if((in = fopen(fileName, "rb")) != NULL){
		if(format == WF_BASE64){
			while(!feof(in)){
				base64Buffer[0] = 0; base64Buffer[1] = 0; base64Buffer[2] = 0;
				bytesRead = fread(base64Buffer, sizeof(char), 3, in);
				idx = base64Buffer[0] >> 2;
				fputc(base64Alphabet[idx], output);
				idx = ((base64Buffer[0] & 0x03) << 4) | (base64Buffer[1] >> 4);
				fputc(base64Alphabet[idx], output);
				if(bytesRead == 2)
					idx = 64;
				else
					idx = ((base64Buffer[1] & 0x0f) << 2) | (base64Buffer[2] >> 6);
				fputc(base64Alphabet[idx], output);
				if(bytesRead <= 2)
					idx = 64;
				else
					idx = base64Buffer[2] & 0x3f;
				fputc(base64Alphabet[idx], output);
				lineLength += 4;
				if(lineLength == 76){
					fputc('\n', output);
					lineLength = 0;
				}
			}
			fputc('\n', output);
		}
		else if(format == WF_SGMLENCODE){
			while((currChr = fgetc(in)) != EOF){
				if(currChr == '&')
					fputs("&amp;", output);
				else if(currChr == '<')
					fputs("&lt;", output);
				else if(currChr == '>')
					fputs("&gt;", output);
				else if(currChr == '\"')
					fputs("&quot;", output);
				else
					fputc(currChr, output);
			}
		}
		else{
			/* Default: verbatim output.  The default handler never returns 
			 * WS_UNSUPPORTED.
			 */
			while((currChr = fgetc(in)) != EOF)
				fputc(currChr, output);
		}
		fclose(in);
	}
	else{
		Logging_warnf("%s: Error opening file \"%s\": %s", 
				__FUNCTION__, fileName, strerror(errno));
		result = WS_ERROR;
	}
	return result;
}
