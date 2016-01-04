#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "FileHandler.h"
#include "astring.h"
#include "Buffer.h"
#include "Logging.h"
#include "memutils.h"
#include "path.h"
#include "stringext.h"


void parseMetaTag(char *buf, Vars *v) {
	Buffer *name  = NULL;
	Buffer *value = NULL;
	int    currState = 0;
	int    nextState = 1;
	char   currChr;
	char   cmpChr;
	char   matchName[]    = "NAME";
	char   matchHttp[]    = "HTTP-EQUIV";
	char   matchContent[] = "CONTENT";
	int    pos = 0;
	bool   getNext = true;
	
	name  = new_Buffer(strlen(buf));
	value = new_Buffer(strlen(buf));
	while (pos < strlen(buf)) {
		getNext = true;
		currChr = buf[pos];
		cmpChr  = toupper(currChr);
		switch (currState) {
			case 0:
			/* Eat whitespace until non-whitespace character encountered */
			if (!isspace(cmpChr)) {
				currState = nextState;
				getNext   = false;
			}
			break;
			
			case 1:
			if (cmpChr == matchName[0])
				currState = 3;
			else if (cmpChr == matchHttp[0])
				currState = 10;
			else if (cmpChr == matchContent[0])
				currState = 20;
			else
				currState = 2;
			break;
			
			case 2:
			/* Eat characters until whitespace character encountered */
			if (isspace(cmpChr)) {
				nextState = 1;
				currState = 0;
			}
			break;
			
			case 3: /* 'A' */
			case 4: /* 'M' */
			case 5: /* 'E' */
			if (cmpChr == matchName[currState - 2])
				currState++;
			else
				currState = 2;
			break;
			
			case 6:
			if (cmpChr == '=')
				currState = 7;
			else if (isspace(cmpChr)) {
				currState = 0;
				nextState = currState;
			}
			else{
				/* Something wrong */
				currState = 2;
			}
			break;

			case 7:
			if (cmpChr == '\"')
				currState = 8;
			else if (isspace(cmpChr)) {
				currState = 0;
				nextState = currState;
			}
			else {
				currState = 9;
				getNext   = false;
			}
			break;
			
			case 8:
			case 9:
			if ((currState == 8 && cmpChr == '\"') || 
				(currState == 9 && isspace(cmpChr))
			) {
				nextState = 1;
				currState = 0;
			}
			else
				Buffer_appendChar(name, currChr);
			break;
			
			case 10: /* 'T' */
			case 11: /* 'T' */
			case 12: /* 'P' */
			case 13: /* '-' */
			case 14: /* 'E' */
			case 15: /* 'Q' */
			case 16: /* 'U' */
			case 17: /* 'I' */
			case 18: /* 'V' */
			if (cmpChr == matchHttp[currState - 9])
				currState++;
			else
				currState = 2;
			break;
			
			case 19:
			currState = 6;
			getNext   = false;
			break;
			
			case 20: /* 'O' */
			case 21: /* 'N' */
			case 22: /* 'T' */
			case 23: /* 'E' */
			case 24: /* 'N' */
			case 25: /* 'T' */
			if (cmpChr == matchContent[currState - 19])
				currState++;
			else
				currState = 2;
			break;
			
			case 26:
			if (cmpChr == '=')
				currState = 27;
			else if (isspace(cmpChr)) {
				currState = 0;
				nextState = currState;
			}
			else{
				/* Something wrong */
				currState = 2;
			}
			break;
			
			case 27:
			if (cmpChr == '\"')
				currState = 28;
			else if (isspace(cmpChr)) {
				currState = 0;
				nextState = currState;
			}
			else {
				currState = 29;
				getNext   = false;
			}
			break;
			
			case 28:
			case 29:
			if ((currState == 28 && cmpChr == '\"') || 
				(currState == 29 && isspace(cmpChr))
			) {
				nextState = 1;
				currState = 0;
			}
			else
				Buffer_appendChar(value, currChr);
			break;
			
			default: /* Shouldn't happen */
			Logging_warnf("%s: Illegal parser state %d", __FUNCTION__, 
				currState);
			break;
		}
		if(getNext) pos++;
	}
	if(strlen(name->data) > 0 && strlen(value->data) > 0) {
		/* Convert variable names to lowercase and remove illegal characters */
		strxlower(name->data);
		strxfilter(name->data, "abcdefghijklmnopqrstuvwxyz0123456789_", '_');
		Vars_let(v, name->data, value->data, VAR_STD);
	}
	delete_Buffer(name);
	delete_Buffer(value);
}

/**
 * \brief Checks the file extension against a list of extensions of files known 
 * to contain HTML.
 */
bool htmlCanHandle(char *fileName) {
	char *fileExt = NULL;
	bool result   = false;
	
	fileExt = getPathPart(fileName, PATH_EXT);
	if (fileExt != NULL) {
		result =
			strxequalsi(fileExt, "asp")   ||
			strxequalsi(fileExt, "aspx")  ||
			strxequalsi(fileExt, "cfm")   ||
			strxequalsi(fileExt, "chtml") || 
			strxequalsi(fileExt, "htm")   ||
			strxequalsi(fileExt, "html")  ||
			strxequalsi(fileExt, "inc")   || /* Not normally processed */
			strxequalsi(fileExt, "jsp")   ||
			strxequalsi(fileExt, "php")   ||
			strxequalsi(fileExt, "php3")  ||
			strxequalsi(fileExt, "phtml") ||
			strxequalsi(fileExt, "stm")   ||
			strxequalsi(fileExt, "shtml")
		;
		mu_free(fileExt);
	}
	return result;
}


/** 
 * \brief Reads the TITLE and META elements in the HTML HEAD element and adds 
 * them as variables. 
 * 
 * \note This is a fairly brute-force HTML scanner (wouldn't dignify it 
 * with the term "parser") but it should be able to handle the 
 * worst sort of tag soup...
 */
void htmlReadMetadata(char *fileName, Vars *data) {
	FILE   *input = NULL;
	int    state = 0;
	bool   keepGoing = true;
	int   currChr;
	char   cmpChr;
	Buffer *buf  = NULL;
	char   headTag[]   = "<HEAD";
	char   headCTag[]  = "/HEAD";
	char   metaTag[]   =  "META";
	char   titleTag[]  = "TITLE";
	
	buf = new_Buffer(0);
	
	if ((input = fopen(fileName, "r")) != NULL) {
		while (keepGoing && (currChr = fgetc(input)) != EOF) {
			cmpChr = toupper(currChr);
			switch (state) {
				case 0: /* < */
				case 1: /* H */
				case 2: /* E */
				case 3: /* A */
				case 4: /* D */
				/* Inside HEAD? */
				if (cmpChr == headTag[state])
					state++;
				else {
					/* Not head tag: Go back to scanning */
					state = 0;
				}
				break;
				
				case 5:
				/* Eat characters until end of tag */
				if(cmpChr == '>') state = 6;
				break;
				
				case 6:
				/* Eat characters until start tag encountered. */
				if(cmpChr == '<') state = 7;
				break;
				
				case 7:
				/* Could be:
				 * a) TITLE tag,
				 * b) META tag,
				 * c) Closing HEAD tag,
				 * d) Something else we're not interested in.
				 */
				if (cmpChr == titleTag[0])
					state = 8;
				else if (cmpChr == metaTag[0])
					state = 14;
				else if (cmpChr == headCTag[0])
					state = 18;
				else
					state = 5;
				break;
				
				case 8:  /* 'I' */
				case 9:  /* 'T' */
				case 10: /* 'L' */
				case 11: /* 'E' */
				if(cmpChr == titleTag[state - 7])
					state++;
				else{
					/* Not TITLE tag; ignore */
					state = 5;
				}
				break;

				case 12:
				/* Eat characters until end of tag */
				if (cmpChr == '>') state = 13;
				break;

				case 13:
				/* Append characters until we come to a '<' character */
				if (cmpChr == '<') {
					Vars_let(data, "title", buf->data, VAR_STD);
					Buffer_reset(buf);
					state = 7;
				}
				else {
					/* Convert all whitespace to spaces */
					if (isspace(currChr)) currChr = ' ';
					Buffer_appendChar(buf, currChr);
				}
				break;
				
				case 14: /* 'E' */
				case 15: /* 'T' */
				case 16: /* 'A' */
				if (cmpChr == metaTag[state - 13])
					state++;
				else {
					/* Not META tag; ignore */
					state = 5;
				}
				break;

				case 17:
				/* Append characters until we come to a '>' */
				if (cmpChr == '>') {
					parseMetaTag(buf->data, data);
					Buffer_reset(buf);
					state = 6;
				}
				else
					Buffer_appendChar(buf, currChr);
				break;
				
				case 18: /* 'H' */
				case 19: /* 'E' */
				case 20: /* 'A' */
				case 21: /* 'D' */
				if (cmpChr == headCTag[state - 17])
					state++;
				else
					state = 5;
				break;

				case 22:
				keepGoing = false;
				break;
				
				default: /* Shouldn't happen! */
				Logging_warnf("%s: Illegal parser state %d", __FUNCTION__, 
					state);
				keepGoing = false;
				break;
			} /* switch(state) */
		} /* while */
		fclose(input);
	}
	else{
		Logging_warnf("%s: Unable to open file \"%s\": %s", __FUNCTION__, 
				fileName, strerror(errno));
	}
	if (!Vars_defined(data, "content_type"))
		Vars_let(data, "content_type", "text/html", VAR_STD);
	delete_Buffer(buf);
}


WriteStatus htmlWriteOutput(char *fileName, WriteFormat format, FILE *output) {
	WriteStatus result = WS_OK;
	FILE   *input = NULL;
	int    state = 0;
	bool   keepGoing = true;
	int    currChr;
	char   cmpChr;
	Buffer *buf  = NULL;
	char   htmlTag[]   = "<HTML";
	char   openTag[] = "<BODY";
	char   closeTag[] = "/BODY";
	
	if (format == WF_HTMLBODY) {
		buf = new_Buffer(0);
		if ((input = fopen(fileName, "r")) != NULL) {
			while (keepGoing && (currChr = fgetc(input)) != EOF) {
				cmpChr = toupper(currChr);
				switch(state) {
					case 0: /* < */
					case 1: /* B */
					case 2: /* O */
					case 3: /* D */
					case 4: /* Y */
					/* Is it a <BODY> tag? */
					if (cmpChr == openTag[state])
						state++;
					else {
						/* Not body tag: Go back to scanning */
						state = 0;
					}
					break;
					
					case 5:
					/* Eat characters until we reach end of tag */
					if (currChr == '>')
						state = 6;
					break;
					
					case 6:
					/* Reached closing tag? */
					if (currChr == '<') {
						Buffer_appendChar(buf, currChr);
						state = 7;
					}
					else
						fputc(currChr, output);
					break;
	
					case 7:  /* / */
					case 8:  /* B */
					case 9:  /* O */
					case 10: /* D */
					case 11: /* Y */
					/* Is it a </BODY> tag? */
					if (cmpChr == closeTag[state - 7]) {
						Buffer_appendChar(buf, currChr);
						state++;
					}
					else {
						/* Not the tag we're looking for:   */
						/* emit tag buffer and reset state. */
						fputs(buf->data, output);
						fputc(currChr, output);
						Buffer_reset(buf);
						state  = 6;
					}
					break;
					
					case 12:
					keepGoing = false;
					break;
					
					default: /* Shouldn't happen */
					Logging_warnf("%s: Illegal parser state %d", __FUNCTION__, 
						state);
					keepGoing = false;
					break;
				}
			}
			delete_Buffer(buf);
			fclose(input);
		}
		else {
			Logging_warnf("%s: Unable to open file \"%s\": %s", __FUNCTION__, 
					fileName, strerror(errno));
		}
	}
	else if (format == WF_HTMLPREAMBLE) {
		if ((input = fopen(fileName, "r")) != NULL) {
			buf = new_Buffer(0);
			while (keepGoing && (currChr = fgetc(input)) != EOF) {
				cmpChr = toupper(currChr);
				switch(state) {
					case 0: /* < */
					case 1: /* H */
					case 2: /* T */
					case 3: /* M */
					case 4: /* L */
						if (cmpChr == htmlTag[state]) {
							Buffer_appendChar(buf, currChr);
							if (state == 4) {
								/* Found opening HTML tag; exit */
								Buffer_reset(buf);
								keepGoing = false;
							}
							state++;
						}
						else {
							/* Not HTML tag; emit buffer and continue */
							if (!strxempty(buf->data)) fputs(buf->data, output);
							Buffer_reset(buf);
							fputc(currChr, output);
							state = 0;
						}
						break;
				}
			}
			if (!strxempty(buf->data)) fputs(buf->data, output);
			delete_Buffer(buf);
			fclose(input);
		}
		else{
			Logging_warnf("%s: Unable to open file \"%s\": %s", __FUNCTION__, 
					fileName, strerror(errno));
		}
	}
	else {
		result = WS_UNSUPPORTED;
	}
	return result;
}
