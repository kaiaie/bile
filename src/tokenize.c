/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: tokenize.c,v 1.4 2006/03/21 23:58:48 ken Exp $
 */
#include <ctype.h>
#include <string.h>
#include "astring.h"
#include "bool.h"
#include "Buffer.h"
#include "Logging.h"
#include "tokenize.h"

List *tokenize(const char *input){
	List   *retVal    = NULL;
	Buffer *currToken = NULL;
	enum {STATE_INITIAL, STATE_DIGITS, STATE_KEYWORD, STATE_STRING};
	int  state = STATE_INITIAL;
	bool redo    = false;
	bool skip    = false;
	bool advance = false;
	bool gotDot  = false;
	char currChar;
	int  ii = 0;
	
	if(input != NULL && strlen(input) > 0){
		retVal    = new_List();
		currToken = new_Buffer(strlen(input));
		while(ii < strlen(input)){
			currChar = input[ii];
			switch(state){
				case STATE_INITIAL:
					/* What kind of token is it? */
					if(isspace(currChar)){
						/* White space: ignore */
						skip = true;
					}
					else if(isdigit(currChar)){
						/* Numeric literal */
						state = STATE_DIGITS;
					}
					else if(isalpha(currChar) || currChar == '$'){
						/* Keyword, variable name or function */
						currChar = (char)tolower(currChar);
						state = STATE_KEYWORD;
					}
					else if(strchr(".+-/*^(),?:=", currChar) != NULL){
						/* Single-character operator */
						advance = true;
					}
					else if(currChar == '`'){
						/* String literal */
						state = STATE_STRING;
					}
					break;
				case STATE_DIGITS:
					if(currChar == '.'){
						if(gotDot){
							skip    = true;
							redo    = true;
							advance = true;
						}
						gotDot = true;
					}
					else if(!isdigit(currChar)){
						skip = true;
						redo = true;
						advance = true;
					}
					break;
				case STATE_KEYWORD:
					if(currChar == '(' || ii == strlen(input) - 1){
						advance = true;						
					}
					else if(!isalnum(currChar) && currChar != '_'){
						skip = true;
						redo = true;
						advance = true;
					}
					break;
				case STATE_STRING:
					if(currChar == '`'){
						advance = true;
					}
					break;
				default:
					Logging_warn("tokenize(): Illegal parse state!");
					state = STATE_INITIAL;
					break;
			}
			if(skip){
				skip = false;
			}
			else{
				Buffer_appendChar(currToken, currChar);
			}
			if(advance){
				if(strlen(currToken->data) > 0){
					List_append(retVal, astrcpy(currToken->data));
					Buffer_reset(currToken);
				}
				state   = STATE_INITIAL;
				advance = false;
			}
			if(redo){
				redo = false;
			}
			else{
				ii++;
			}
		}
		delete_Buffer(currToken);
	}
	return retVal;
}

