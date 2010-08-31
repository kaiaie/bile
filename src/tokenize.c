/* :tabSize=4:indentSize=4:folding=indent:
** $Id: tokenize.c,v 1.8 2010/08/31 15:11:58 ken Exp $
*/
#include <ctype.h>
#include <string.h>
#include "astring.h"
#include "bool.h"
#include "Buffer.h"
#include "Logging.h"
#include "memutils.h"
#include "tokenize.h"

/** Converts the input string into a list of tokens */
List *tokenize(const char *input) {
	List   *retVal    = NULL;
	Buffer *currToken = NULL;
	enum {STATE_INITIAL, STATE_DIGITS, STATE_VARIABLE, STATE_KEYWORD, STATE_STRING};
	int  state = STATE_INITIAL;
	bool redo    = false; /* Set to true if this character is part of a subsequent token */
	bool skip    = false; /* Set to true if this character is not to be added to the current token */
	bool advance = false; /* Set to true to indicate a complete token has been formed */
	bool gotDot  = false;
	char currChar;
	char quoteChar = '`';
	char *tmp = NULL;
	int  ii = 0;
	
	if (input != NULL && strlen(input) > 0) {
		retVal    = new_List();
		currToken = new_Buffer(strlen(input));
		tmp = astrcat(input, " ");
		while (ii < strlen(tmp)) {
			currChar = tmp[ii];
			switch (state) {
				case STATE_INITIAL:
					/* What kind of token is it? */
					if (isspace(currChar)) {
						/* White space: ignore */
						skip = true;
					}
					else if (isdigit(currChar)) {
						/* Numeric literal */
						state = STATE_DIGITS;
					}
					else if (currChar == '$') {
						state = STATE_VARIABLE;
					}
					else if (isalpha(currChar)) {
						/* Keyword, variable name or function */
						currChar = (char)tolower(currChar);
						state = STATE_KEYWORD;
					}
					else if (strchr(".+-/*^(),?:=", currChar) != NULL) {
						/* Single-character operator */
						advance = true;
					}
					else if (strchr("`'\"", currChar) != NULL) {
						/* String literal */
						quoteChar = currChar;
						state = STATE_STRING;
					}
					break;
				case STATE_DIGITS:
					if (currChar == '.') {
						if (gotDot) {
							skip    = true;
							redo    = true;
							advance = true;
						}
						gotDot = true;
					}
					else if (!isdigit(currChar)) {
						skip = true;
						redo = true;
						advance = true;
					}
					break;
				case STATE_VARIABLE:
					if (!isalnum(currChar) && currChar != '_' && currChar != '$') {
						skip = true;
						redo = true;
						advance = true;
					}
					else {
						state = STATE_KEYWORD;
					}
					break;
				case STATE_KEYWORD:
					if (currChar == '(') {
						advance = true;						
					}
					else if (!isalnum(currChar) && currChar != '_') {
						skip = true;
						redo = true;
						advance = true;
					}
					break;
				case STATE_STRING:
					if (currChar == quoteChar) {
						advance = true;
					}
					break;
				default:
					Logging_warn("tokenize(): Illegal parse state!");
					state = STATE_INITIAL;
					break;
			}
			if (skip) {
				skip = false;
			}
			else {
				Buffer_appendChar(currToken, currChar);
			}
			if (advance) {
				if (strlen(currToken->data) > 0) {
					List_append(retVal, astrcpy(currToken->data));
					Buffer_reset(currToken);
				}
				state   = STATE_INITIAL;
				advance = false;
			}
			if (redo) {
				redo = false;
			}
			else {
				ii++;
			}
		}
		mu_free(tmp);
		delete_Buffer(currToken);
	}
	return retVal;
}

