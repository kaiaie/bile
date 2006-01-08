/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: ImgHandler.c,v 1.2 2006/01/08 18:02:54 ken Exp $
 */
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "Buffer.h"
#include "ImgHandler.h"
#include "Logging.h"
#include "path.h"
#include "stringext.h"

#define gifHeaderLength 6
#define pngHeaderLength 8


typedef struct _pngChunk{
	size_t        length;
	char          type[5];
	unsigned char *data;
	long          crc;
} PngChunk;


int wordLeToInt(unsigned char *buffer){
	int result = 0;
	
	result = buffer[1]; result <<= 8; result += buffer[0];
	return result;
}


int wordBeToInt(unsigned char *buffer){
	int result = 0;
	
	result = buffer[0]; result <<= 8; result += buffer[1];
	return result;
}


long dwordBeToLong(unsigned char *buffer){
	long result = 0;
	
	result = buffer[0]; result <<= 8;
	result += buffer[1]; result <<= 8;
	result += buffer[2]; result <<= 8;
	result += buffer[3];
	return result;
}


int readWordLe(FILE *input){
	unsigned char buffer[2];
	int           result = 0;
	
	if(fread(buffer, sizeof(char), 2, input)){
		result = wordLeToInt(buffer);
	}
	else
		Logging_warnf("%s: Premature end-of-file encountered.", __FUNCTION__);
	return result;
}


long readDwordBe(FILE *input){
	unsigned char buffer[4];
	long          result = 0;
	
	if(fread(buffer, sizeof(char), 4, input)){
		result = dwordBeToLong(buffer);
	}
	else
		Logging_warnf("%s: Premature end-of-file encountered.", __FUNCTION__);
	return result;
}


PngChunk *getChunk(FILE *input){
	PngChunk      *result    = NULL;
	
	if((result = (PngChunk *)malloc(sizeof(PngChunk))) != NULL){
		result->length = (size_t)readDwordBe(input);
		fread(result->type, sizeof(char), 4, input);
		result->type[4] = '\0';
		if((result->data = (unsigned char *)malloc(result->length)) != NULL){
			fread(result->data, sizeof(char), result->length, input);
			result->crc = readDwordBe(input);
		}
		else{
			free(result);
			Logging_fatalf("%s: Out of memory!", __FUNCTION__);
		}
	}
	else
		Logging_fatalf("%s: Out of memory!", __FUNCTION__);
	return result;
}


unsigned char *getJpegData(FILE *input){
	unsigned char *result = NULL;
	unsigned char buffer[4];
	long          currPos;
	size_t        resultLen;
	
	currPos = ftell(input);
	if(fread(buffer, sizeof(char), 4, input) == 4){
		/* Is it a marker byte? */
		if(buffer[0] == 0xff){
			resultLen = wordBeToInt(&buffer[2]) + 2;
			/* Back up so we include the marker in the buffer */
			fseek(input, currPos, SEEK_SET);
			if((result = (unsigned char *)malloc(resultLen)) != NULL){
				if(fread(result, sizeof(char), resultLen, input) != resultLen){
					Logging_warnf("%s: Premature end of file", __FUNCTION__);
					free(result);
					result = NULL;
				}
			}
			else
				Logging_fatalf("%s: Out of memory!", __FUNCTION__);
		}
	}
	return result;
}


void readGif(FILE *input, Dict *d){
	unsigned char header[gifHeaderLength];
	
	if(fread(header, sizeof(char), gifHeaderLength, input) == gifHeaderLength){
		if(strncmp(header, "GIF87a", 6) == 0 || strncmp(header, "GIF89a", 6) == 0){
			/* GIF is little-endian and height and width are 2 bytes wide */
			Dict_put(d, "image_width",  asprintf("%d", readWordLe(input)));
			Dict_put(d, "image_height", asprintf("%d", readWordLe(input)));
			/* TODO: Add code to extract comments */
		}
		else
			Logging_warnf("%s: Not a valid GIF file.", __FUNCTION__);
	}
	else
		Logging_warnf("%s: Error reading GIF header; file is too short.", __FUNCTION__);
}


void readJpg(FILE *input, Dict *d){
	unsigned char buffer[2];
	unsigned char *data = NULL;
	Buffer        *comments = NULL;
	size_t        dataLen;
	
	comments = new_Buffer(0);
	/* Check for Start Of Image (SOI) marker */
	if(fread(buffer, sizeof(char), 2, input) == 2){
		if(buffer[0] == 0xff && buffer[1] == 0xd8){
			data = getJpegData(input);
			if(data != NULL && /* Got marker */
					data[0] == 0xff && data[1] == 0xe0 && /* Got APP0 */
					strequals(&data[4], "JFIF") /* is JFIF file */){
				free(data);
				while((data = getJpegData(input)) != NULL){
					if(data[0] == 0xff && data[1] == 0xc0){ /* SOF0 marker */
						Dict_put(d, "image_height",
								asprintf("%d", wordBeToInt(&data[5])));
						Dict_put(d, "image_width",
								asprintf("%d", wordBeToInt(&data[7])));
					}
					else if(data[0] == 0xff && data[1] == 0xfe) { /* Comment */
						dataLen = wordBeToInt(&data[2]);
						if(strlen(comments->data) != 0){
							Buffer_appendChar(comments, '\n');
						}
						Buffer_appendChars(comments, &data[4], dataLen - 2);
					}
					free(data);
				}
				if(strlen(comments->data) > 0 && !Dict_exists(d, "comments")){
					Dict_put(d, "comments", astrcpy(comments->data));
				}
				delete_Buffer(comments);
			}
			else
				Logging_warnf("%s: Invalid JPEG file; missing or invalid APP0", __FUNCTION__);
		}
		else
			Logging_warnf("%s: Invalid JPEG file; no SOI marker", __FUNCTION__);
	}
	else
		Logging_warnf("%s: Error reading JPEG file; file is too short.", __FUNCTION__);
}


void readPng(FILE *input, Dict *d){
	unsigned char header[pngHeaderLength];
	PngChunk *chunk = NULL;
	char     *name = NULL;
	char     *text = NULL;
	
	if(fread(header, sizeof(char), pngHeaderLength, input) == pngHeaderLength){
		if(header[0] == 0x89 && header[1] == 0x50 && header[2] == 0x4e
				&& header[3] == 0x47 && header[4] == 0x0d && header[5] == 0x0a
				&& header[6] == 0x1a && header[7] == 0x0a){
			/* Get metadata
			 * Image dimensions are in the IHDR chunk
			 * Text data are in the tEXt chunk
			 */
			while(!feof(input) && (chunk = getChunk(input)) != NULL){
				if(strequals(chunk->type, "IHDR")){
					/* PNG is big-endian and height and width are 4 bytes wide */
					Dict_put(d, "image_width",  asprintf("%ld", dwordBeToLong(chunk->data)));
					Dict_put(d, "image_height", asprintf("%ld", dwordBeToLong(&chunk->data[4])));
				}
				else if(strequals(chunk->type, "tEXt")){
					/* PNG tEXt chunks consist of a null-separated 
					 * name/value pair.
					 */
					name = astrcpy((char *)chunk->data);
					/* Lower case and remove illegal characters */
					strlower(name);
					strfilter(name, "abcdefghijklmnopqrstuvwxyz0123456789_", '_');
					if(!Dict_exists(d, name)){
						text = (char *)&chunk->data[strlen(name)];
						Dict_put(d, name, astrcpy(text));
					}
					free(name);
				}
				if(chunk->data != NULL) free(chunk->data);
				free(chunk);
			}
			
		}
		else
			Logging_warnf("%s: Not a valid PNG file.", __FUNCTION__);
	}
	else
		Logging_warnf("%s: Error reading PNG header; file is too short.", __FUNCTION__);
}


bool imgCanHandle(char *fileName){
	bool result   = false;
	char *fileExt = NULL;
	
	/* TODO: add better checks than just looking at file extension. */
	fileExt = getPathPart(fileName, PATH_EXT);
	if(fileExt != NULL){
		result = strequalsi(fileExt, "gif") || 
				strequalsi(fileExt, "jpg") || 
				strequalsi(fileExt, "jpeg") || 
				strequalsi(fileExt, "png");
		free(fileExt);
	}
	return result;
}


void imgReadMetadata(char *fileName, Dict *data){
	char *fileExt = NULL;
	FILE *input   = NULL;
	
	fileExt = getPathPart(fileName, PATH_EXT);
	if(fileExt != NULL){
		if((input = fopen(fileName, "rb")) != NULL){
			if(strequalsi(fileExt, "gif"))
				readGif(input, data);
			else if(strequalsi(fileExt, "jpg") || strequalsi(fileExt, "jpeg"))
				readJpg(input, data);
			else if(strequalsi(fileExt, "png"))
				readPng(input, data);
			fclose(input);
		}
		else{
			Logging_warnf("%s: Error opening file \"%s\": %s", __FUNCTION__, 
					fileName, strerror(errno));
		}
		free(fileExt);
	}
}


WriteStatus imgWriteOutput(char *fileName, WriteFormat format, FILE *output){
	return WS_UNSUPPORTED;
}

