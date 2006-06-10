/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: ImgHandler.c,v 1.7 2006/06/10 20:23:42 ken Exp $
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "Buffer.h"
#include "ImgHandler.h"
#include "Logging.h"
#include "memutils.h"
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
	
	if(fread(buffer, sizeof(char), 2, input))
		result = wordLeToInt(buffer);
	else
		Logging_warnf("%s(): Premature end-of-file encountered.", __FUNCTION__);
	return result;
}


long readDwordBe(FILE *input){
	unsigned char buffer[4];
	long          result = 0;
	
	if(fread(buffer, sizeof(char), 4, input))
		result = dwordBeToLong(buffer);
	else
		Logging_warnf("%s(): Premature end-of-file encountered.", __FUNCTION__);
	return result;
}


PngChunk *getChunk(FILE *input){
	PngChunk      *result    = NULL;
	
	result = (PngChunk *)mu_malloc(sizeof(PngChunk));
	result->length = (size_t)readDwordBe(input);
	fread(result->type, sizeof(char), 4, input);
	result->type[4] = '\0';
	result->data = (unsigned char *)mu_malloc(result->length);
	fread(result->data, sizeof(char), result->length, input);
	result->crc = readDwordBe(input);
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
			result = (unsigned char *)mu_malloc(resultLen);
			if(fread(result, sizeof(char), resultLen, input) != resultLen){
				Logging_warnf("%s: Premature end of file", __FUNCTION__);
				mu_free(result);
				result = NULL;
			}
		}
	}
	return result;
}


void readGif(FILE *input, Vars *v){
	unsigned char header[gifHeaderLength];
	int packedFlags;
	int extChar;
	int extType;
	int subBlockSize;
	int commentChar;
	long offset;
	int ii;
	char *tmp = NULL;
	Buffer *comments = NULL;
	
	Vars_let(v, "content_type", "image/gif", VAR_STD);
	if(fread(header, sizeof(char), gifHeaderLength, input) == gifHeaderLength){
		if(strncmp(header, "GIF87a", gifHeaderLength) == 0 || 
			strncmp(header, "GIF89a", gifHeaderLength) == 0){
			/* GIF is little-endian and height and width are 2 bytes wide */
			tmp = asprintf("%d", readWordLe(input));
			Vars_let(v, "image_width", tmp, VAR_STD);
			mu_free(tmp);
			tmp = asprintf("%d", readWordLe(input));
			Vars_let(v, "image_height", tmp, VAR_STD);
			mu_free(tmp);
			/* Look for comment block (only in GIF89a) */
			if(strncmp(header, "GIF89a", gifHeaderLength) == 0){
				comments = new_Buffer(0);
				if((packedFlags = fgetc(input)) != EOF){
					offset = 2; /* Two fields in the Logical Screen Descriptor we don't care about */
					/* Bit 7 set if we have a GCT */
					if(packedFlags & 0x80){
						/* Bits 0-2 are the number of entries in the GCT */
						packedFlags &= 0x7;
						offset += (1L << (packedFlags + 1)) * 3;
					}
					fseek(input, offset, SEEK_CUR);
					/* Is this an extension block? */
					while(true){
						if((extChar = fgetc(input)) != EOF && extChar == 0x21){
							if((extType = fgetc(input)) != EOF){
								if(extType == 0xf9){
									/* Graphics control block; skip it */
									/* (Always 6 bytes long) */
									fseek(input, 6, SEEK_CUR);
								}
								else if(extType == 0x01 || extType == 0xff || extType == 0xfe){
									/* Variable-sized extension blocks
									 * We're only interested in the Comment block (type 0xfe)
									 * but we have to read the other kinds of block too in 
									 * order to skip over them.
									 */
									if(extType == 0x01) fseek(input, 14, SEEK_CUR);
									else if(extType == 0xff) fseek(input, 12, SEEK_CUR);
									while((subBlockSize = fgetc(input)) != EOF && subBlockSize != 0){
										if(extType == 0xfe){
											if(strlen(comments->data) > 0) Buffer_appendChar(comments, '\n');
											for(ii = 0; ii < subBlockSize; ++ii){
												if((commentChar = fgetc(input)) != EOF)
													Buffer_appendChar(comments, commentChar);
												else break;
											}
										}
										else
											fseek(input, subBlockSize, SEEK_CUR); /* Discard data */
									}
								}
								else{
									Logging_warnf("Unrecognised GIF Extension block type.");
									break;
								}
							}
							else break;
						}
						else break;
					}
					/* Add comments if we found any */
					if(strlen(comments->data) > 0){
						Vars_let(v, "comments", comments->data, VAR_STD);
					}
					delete_Buffer(comments);
				}
			}
		}
		else
			Logging_warnf("%s: Not a valid GIF file.", __FUNCTION__);
	}
	else
		Logging_warnf("%s: Error reading GIF header; file is too short.", __FUNCTION__);
}


void readJpg(FILE *input, Vars *v){
	unsigned char buffer[2];
	unsigned char *data = NULL;
	Buffer        *comments = NULL;
	size_t        dataLen;
	char          *tmp = NULL;
	
	Vars_let(v, "content_type", "image/jpeg", VAR_STD);
	comments = new_Buffer(0);
	/* Check for Start Of Image (SOI) marker */
	if(fread(buffer, sizeof(char), 2, input) == 2){
		if(buffer[0] == 0xff && buffer[1] == 0xd8){
			data = getJpegData(input);
			if(data != NULL && /* Got marker */
					data[0] == 0xff && data[1] == 0xe0 && /* Got APP0 */
					strequals(&data[4], "JFIF") /* is JFIF file */){
				mu_free(data);
				while((data = getJpegData(input)) != NULL){
					if(data[0] == 0xff && data[1] == 0xc0){ /* SOF0 marker */
						tmp = asprintf("%d", wordBeToInt(&data[5]));
						Vars_let(v, "image_height", tmp, VAR_STD);
						mu_free(tmp);
						tmp = asprintf("%d", wordBeToInt(&data[7]));
						Vars_let(v, "image_width", tmp, VAR_STD);
						mu_free(tmp);
					}
					else if(data[0] == 0xff && data[1] == 0xfe) { /* Comment */
						dataLen = wordBeToInt(&data[2]);
						if(strlen(comments->data) != 0){
							Buffer_appendChar(comments, '\n');
						}
						Buffer_appendChars(comments, &data[4], dataLen - 2);
					}
					mu_free(data);
				}
				if(strlen(comments->data) > 0 && !Vars_defined(v, "comments")){
					Vars_let(v, "comments", comments->data, VAR_STD);
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


void readPng(FILE *input, Vars *v){
	unsigned char header[pngHeaderLength];
	PngChunk *chunk = NULL;
	char     *name = NULL;
	char     *text = NULL;
	char     *tmp  = NULL;
	
	Vars_let(v, "content_type", "image/png", VAR_STD);
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
					tmp = asprintf("%ld", dwordBeToLong(chunk->data));
					Vars_let(v, "image_width",  tmp, VAR_STD);
					mu_free(tmp);
					tmp = asprintf("%ld", dwordBeToLong(&chunk->data[4]));
					Vars_let(v, "image_height", tmp, VAR_STD);
					mu_free(tmp);
				}
				else if(strequals(chunk->type, "tEXt")){
					/* PNG tEXt chunks consist of a null-separated 
					 * name/value pair.
					 */
					name = astrcpy((char *)chunk->data);
					/* Lower case and remove illegal characters */
					strlower(name);
					strfilter(name, "abcdefghijklmnopqrstuvwxyz0123456789_", '_');
					text = (char *)&chunk->data[strlen(name)];
					Vars_let(v, name, text, VAR_STD);
					mu_free(name);
				}
				mu_free(chunk->data);
				mu_free(chunk);
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
		mu_free(fileExt);
	}
	return result;
}


void imgReadMetadata(char *fileName, Vars *data){
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
		mu_free(fileExt);
	}
}


WriteStatus imgWriteOutput(char *fileName, WriteFormat format, FILE *output){
	return WS_UNSUPPORTED;
}

