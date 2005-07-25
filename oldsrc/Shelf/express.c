#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "fns.h"
#include "global.h"
#include "list.h"
#include "stack.h"
#include "str.h"
#include "var.h"

typedef enum{
	FN_NOP,             FN_ABS,             FN_ACS, 
	FN_ASN,             FN_ATN,             FN_BY, 
	FN_CAT,             FN_COS,             FN_CURRENT, 
	FN_DATE_DIFF,       FN_DAY,             FN_DECODE, 
	FN_DIV,             FN_ENTITY,          FN_EQ, 
	FN_EVAL,            FN_EXEC,            FN_EXP, 
	FN_FILE,            FN_FILE_CDATE,      FN_FILE_EXT, 
	FN_FILE_MDATE,      FN_FILE_NAME_ONLY,  FN_FILE_PATH, 
	FN_FILE_SIZE,       FN_FILE_TYPE,       FN_FORMAT_DATE, 
	FN_FORMAT_NUMBER,   FN_GE,              FN_GET_ITEM, 
	FN_GET_SETTING,     FN_GT,              FN_HOUR, 
	FN_HTML,            FN_IIF,             FN_IMAGE_COLORS, 
	FN_IMAGE_COMMENTS,  FN_IMAGE_HEIGHT,    FN_IMAGE_WIDTH, 
	FN_INITCAP,         FN_INSTR,           FN_INT, 
	FN_ITEM_COUNT,      FN_LCASE,           FN_LE, 
	FN_LEN,             FN_LESS,            FN_LN, 
	FN_LT,              FN_LTRIM,           FN_MAX, 
	FN_MIN,             FN_MINUTE,          FN_MOD, 
	FN_MONTH,           FN_NE,              FN_NEG, 
	FN_NOW,             FN_OVER,            FN_PLAIN, 
	FN_PLUS,            FN_POW,             FN_RND,             FN_RTRIM, 
	FN_SECOND,          FN_SIN,             FN_SQRT, 
	FN_SUBSTR,          FN_TAG,             FN_TAN, 
	FN_TO_DATE,         FN_TRIM,            FN_UCASE, 
	FN_URL,             FN_VAR,             FN_YEAR
} fnId;
typedef enum{OP_PREFIX, OP_INFIX} opType;

struct fnProps{
	fnId   id;
	char   name[20];
	opType type;
};

struct fnProps fnList[] = {
	{FN_ABS,             "abs",              OP_PREFIX}, 
	{FN_ACS,             "acs",              OP_PREFIX}, 
	{FN_ASN,             "asn",              OP_PREFIX}, 
	{FN_ATN,             "atn",              OP_PREFIX}, 
	{FN_BY,              "*",                OP_INFIX}, 
	{FN_CAT,             ".",                OP_INFIX}, 
	{FN_COS,             "cos ",             OP_PREFIX}, 
	{FN_CURRENT,         "current",          OP_PREFIX}, 
	{FN_DATE_DIFF,       "date_diff",        OP_PREFIX}, 
	{FN_DAY,             "day",              OP_PREFIX}, 
	{FN_DECODE,          "decode",           OP_PREFIX}, 
	{FN_DIV,             "div",              OP_INFIX}, 
	{FN_ENTITY,          "entity",           OP_PREFIX}, 
	{FN_EQ,              "eq",               OP_INFIX}, 
	{FN_EVAL,            "eval",             OP_PREFIX}, 
	{FN_EXEC,            "exec",             OP_PREFIX}, 
	{FN_EXP,             "exp",              OP_PREFIX}, 
	{FN_FILE,            "file",             OP_PREFIX}, 
	{FN_FILE_CDATE,      "file_cdate",       OP_PREFIX}, 
	{FN_FILE_EXT,        "file_ext",         OP_PREFIX}, 
	{FN_FILE_MDATE,      "file_mdate",       OP_PREFIX}, 
	{FN_FILE_NAME_ONLY,  "file_name_only",   OP_PREFIX}, 
	{FN_FILE_PATH,       "file_path",        OP_PREFIX}, 
	{FN_FILE_SIZE,       "file_size",        OP_PREFIX}, 
	{FN_FILE_TYPE,       "file_type",        OP_PREFIX}, 
	{FN_FORMAT_DATE,     "format_date",      OP_PREFIX}, 
	{FN_FORMAT_NUMBER,   "format_number",    OP_PREFIX}, 
	{FN_GE,              "ge",               OP_INFIX}, 
	{FN_GET_ITEM,        "get_item",         OP_PREFIX}, 
	{FN_GET_SETTING,     "get_setting",      OP_PREFIX}, 
	{FN_GT,              "gt",               OP_INFIX}, 
	{FN_HOUR,            "hour",             OP_PREFIX}, 
	{FN_HTML,            "html",             OP_PREFIX}, 
	{FN_IIF,             "iif",              OP_PREFIX}, 
	{FN_IMAGE_COLORS,    "image_colors",     OP_PREFIX}, 
	{FN_IMAGE_COMMENTS,  "image_comments",   OP_PREFIX}, 
	{FN_IMAGE_HEIGHT,    "image_height",     OP_PREFIX}, 
	{FN_IMAGE_WIDTH ,    "image_width ",     OP_PREFIX}, 
	{FN_INITCAP ,        "initcap ",         OP_PREFIX}, 
	{FN_INSTR,           "instr",            OP_PREFIX}, 
	{FN_INT,             "int",              OP_PREFIX}, 
	{FN_ITEM_COUNT,      "item_count",       OP_PREFIX}, 
	{FN_LCASE ,          "lcase ",           OP_PREFIX}, 
	{FN_LE,              "le",               OP_PREFIX}, 
	{FN_LEN,             "len",              OP_PREFIX}, 
	{FN_LESS,            "-",                OP_INFIX}, 
	{FN_LN,              "ln",               OP_PREFIX}, 
	{FN_LT,              "lt",               OP_PREFIX}, 
	{FN_LTRIM,           "ltrim",            OP_PREFIX}, 
	{FN_MAX,             "max",              OP_PREFIX}, 
	{FN_MIN,             "min",              OP_PREFIX}, 
	{FN_MINUTE,          "minute",           OP_PREFIX}, 
	{FN_MOD,             "mod",              OP_INFIX}, 
	{FN_MONTH,           "month",            OP_PREFIX}, 
	{FN_NE,              "ne",               OP_INFIX}, 
	{FN_NEG,             "-",                OP_PREFIX}, 
	{FN_NOW,             "now",              OP_PREFIX}, 
	{FN_OVER,            "/",                OP_INFIX}, 
	{FN_PLAIN,           "plain",            OP_PREFIX}, 
	{FN_PLUS,            "+",                OP_INFIX}, 
	{FN_POW,             "^",                OP_INFIX}, 
	{FN_RND,             "rnd",              OP_PREFIX}, 
	{FN_RTRIM,           "rtrim",            OP_PREFIX}, 
	{FN_SECOND,          "second",           OP_PREFIX}, 
	{FN_SIN,             "sin",              OP_PREFIX}, 
	{FN_SQRT,            "sqrt",             OP_PREFIX}, 
	{FN_SUBSTR,          "substr",           OP_PREFIX}, 
	{FN_TAG,             "tag",              OP_PREFIX}, 
	{FN_TAN,             "tan",              OP_PREFIX}, 
	{FN_TO_DATE,         "to_date",          OP_PREFIX}, 
	{FN_TRIM,            "trim",             OP_PREFIX}, 
	{FN_UCASE,           "ucase",            OP_PREFIX}, 
	{FN_URL,             "url",              OP_PREFIX}, 
	{FN_VAR,             "var",              OP_PREFIX}, 
	{FN_YEAR,            "year",             OP_PREFIX}, 
	{FN_NOP,             NULL,               0}
};

bool tokenize(String expr, List l){
   String tmp;
   int parenCount = 0;
   int currState  = 0;
   int lastState  = 0;
   size_t len;
   size_t pos;
   char currChr;
   bool retVal = false;
   
   if(String_getLength(expr, &len)){
      pos = 0;
      while(pos < len){
         retVal = String_getCharAt(expr, pos, &currChr);
         if(currState == 0){
            if(currChr != ' '){
               if((tmp = new_String(NULL)) == INVALID_STRING){
                  retVal = false;
                  break;
               }
               if(currChr == '(' || currChr == ')' || currChr == ','){
                  if((retVal = String_appendChar(tmp, currChr)) == false){
                     break;
                  }
                  if((retVal = List_appendItem(l, (int)tmp)) == false){
                     break;
                  }
                  pos++;
               }
               else{
                  if(currChr == '\"'){
                     currState = 2;
                  }
                  else if(currChr == '\''){
                     currState = 3;
                  }
                  else{
                     currState = 1;
                  }
               }
            }
            else{
               pos++;
            }
         }
         else if(currState == 1){
            if(currChr == ' ' || currChr == '(' || currChr == ')'){
               currState = 5;
            }
            else{
               if(!String_appendChar(tmp, tolower(currChr))){
                  retVal = false;
                  break;
               }
               pos++;
            }
         }
         else if(currState == 2){
            if(currChr == '\\'){
               lastState = 2;
               currState = 4;
               pos++;
            }
            else{
               if((retVal = String_appendChar(tmp, currChr)) == false){
                  break;
               }
               if(currChr == '\"'){
                  currState = 5;
               }
               else{
                  pos++;
               }
            }
         }
         else if(currState == 3){
            if(currChr == '\\'){
               lastState = 3;
               currState = 4;
               pos++;
            }
            else{
               if((retVal = String_appendChar(tmp, currChr)) == false){
                  break;
               }
               if(currChr == '\''){
                  currState = 5;
               }
               else{
                  pos++;
               }
            }
         }
         else if(currState == 4){
            if(currChr == 'n'){
               if((retVal = String_appendChar(tmp, '\n')) == false){
                  break;
               }
            }
            else if(currChr == 't'){
               if((retVal = String_appendChar(tmp, '\t')) == false){
                  break;
               }
            }
            else if(currChr == '\''){
               if((retVal = String_appendChar(tmp, '\'')) == false){
                  break;
               }
            }
            else if(currChr == '\"'){
               if((retVal = String_appendChar(tmp, '\"')) == false){
                  break;
               }
            }
            else if(currChr == '\\'){
               if((retVal = String_appendChar(tmp, '\\')) == false){
                  break;
               }
            }
            else{
               if((retVal = String_appendChar(tmp, '\\')) == false){
                  break;
               }
               if((retVal = String_appendChar(tmp, currChr)) == false){
                  break;
               }
            }
            currState = lastState;
            pos++;
         }
         else if(currState == 5){
            if(currChr == '('){
               parenCount++;
            }
            if(currChr == ')'){
               parenCount--;
            }
            if((retVal = List_appendItem(l, (int)tmp)) == false){
               break;
            }
            currState = 0;
         }
      }
   }
   return retVal && (parenCount == 0);
}


bool parse(Context ctx, List tokens, Stack ops, Stack args, size_t pos){
   int    state  = 0;
   size_t i, j, k;
   int    argCount = 1;
   Stack  tmpOps;
   bool   found;
   bool   retVal = true;
   String currToken;
   char   *tokenChars = NULL;
   
   tmpOps = new_Stack();
   List_getLength(tokens, &j);
   for(i = pos; i < j; ++i){
      List_getItem(tokens, i, (int *)&currToken);
      String_getChars(currToken, &tokenChars);
      if(strcmp(tokenChars, "(") == 0){
         if(i != (j - 1)){
            List_getItem(tokens, (i + 1), (int *)&currToken);
            String_getChars(currToken, &tokenChars);
            if(strcmp(tokenChars, ")") == 0){
               /* Special case: empty parentheses */
            }
            else{
               retVal = parse(ctx, tokens, ops, args, i + 1);
            }
         }
         else{
            /* TODO: Parse error: unmatched parenthesis       */
            /* (most should be caught during tokenisation)... */
            return false;
         }
      }
      if(strcmp(tokenChars, ")") == 0){
         return retVal;
      }
      if(state == 0 || state == 2){
         /* Looking for:                                             */
         /* 1) prefix operator                                       */
         /* 2) variable name                                         */
         /* 3) string or number literal                              */
         k = 0;
         found = false;
         while(fnList[k].name != NULL){
            if(strcmp(fnList[k].name, tokenChars) == 0){
               if(fnList[k].type == OP_PREFIX){
                  Stack_push(tmpOps, (int)fnList[k].id);
                  found = true;
                  break;
               }
            }
            k++;
         }
         if(!found){
            /* Syntax error: infix operator in wrong place */
         }
         if((state == 0) && (i != (j - 1))){
            state = 1;
         }
         else{
            /* End of expression */
         }
      }
      else if(state == 1){
         /* Looking for:                                             */
         /* 1) Argument separator                                    */
         /* 2) Infix operator                                        */
         if(strcmp(tokenChars, ",") == 0){
            argCount++;
            state = 0;
         }
         else{
            k = 0;
            while(true){
               if(strcmp(fnList[k].name, tokenChars) == 0){
                  if(fnList[k].type == OP_INFIX){
                     Stack_push(tmpOps, (int)fnList[k].id);
                     break;
                  }
                  else{
                     /* Syntax error: prefix operator in wrong place */
                  }
               }
               k++;
            }
            state = 2;
         }
      }
   }
   
   return retVal;
}


bool eval(Context ctx, Stack ops, Stack args, Var *result){
   fnId theFunction;
   bool retVal = true;
   
   while(Stack_pop(ops, (int *)&theFunction)){
      switch(theFunction){
         case FN_ABS :
         	fn_abs(ctx, args);
         	break;
         case FN_ACS :
         	fn_acs(ctx, args);
         	break;
         case FN_ASN :
         	fn_asn(ctx, args);
         	break;
         case FN_ATN :
         	fn_atn(ctx, args);
         	break;
         case FN_BY :
         	fn_by(ctx, args);
         	break;
         case FN_CAT :
         	fn_cat(ctx, args);
         	break;
         case FN_COS :
         	fn_cos(ctx, args);
         	break;
         case FN_CURRENT :
         	fn_current(ctx, args);
         	break;
         case FN_DATE_DIFF :
         	fn_date_diff(ctx, args);
         	break;
         case FN_DAY :
         	fn_day(ctx, args);
         	break;
         case FN_DECODE :
         	fn_decode(ctx, args);
         	break;
         case FN_DIV :
         	fn_div(ctx, args);
         	break;
         case FN_ENTITY :
         	fn_entity(ctx, args);
         	break;
         case FN_EQ :
         	fn_eq(ctx, args);
         	break;
         case FN_EVAL :
         	fn_eval(ctx, args);
         	break;
         case FN_EXEC :
         	fn_exec(ctx, args);
         	break;
         case FN_EXP :
         	fn_exp(ctx, args);
         	break;
         case FN_FILE :
         	fn_file(ctx, args);
         	break;
         case FN_FILE_CDATE :
         	fn_file_cdate(ctx, args);
         	break;
         case FN_FILE_EXT :
         	fn_file_ext(ctx, args);
         	break;
         case FN_FILE_MDATE :
         	fn_file_mdate(ctx, args);
         	break;
         case FN_FILE_NAME_ONLY :
         	fn_file_name_only(ctx, args);
         	break;
         case FN_FILE_PATH :
         	fn_file_path(ctx, args);
         	break;
         case FN_FILE_SIZE :
         	fn_file_size(ctx, args);
         	break;
         case FN_FILE_TYPE :
         	fn_file_type(ctx, args);
         	break;
         case FN_FORMAT_DATE :
         	fn_format_date(ctx, args);
         	break;
         case FN_FORMAT_NUMBER :
         	fn_format_number(ctx, args);
         	break;
         case FN_GE :
         	fn_ge(ctx, args);
         	break;
         case FN_GET_ITEM :
         	fn_get_item(ctx, args);
         	break;
         case FN_GET_SETTING :
         	fn_get_setting(ctx, args);
         	break;
         case FN_GT :
         	fn_gt(ctx, args);
         	break;
         case FN_HOUR :
         	fn_hour(ctx, args);
         	break;
         case FN_HTML :
         	fn_html(ctx, args);
         	break;
         case FN_IIF :
         	fn_iif(ctx, args);
         	break;
         case FN_IMAGE_COLORS :
         	fn_image_colors(ctx, args);
         	break;
         case FN_IMAGE_COMMENTS :
         	fn_image_comments(ctx, args);
         	break;
         case FN_IMAGE_HEIGHT :
         	fn_image_height(ctx, args);
         	break;
         case FN_IMAGE_WIDTH :
         	fn_image_width(ctx, args);
         	break;
         case FN_INITCAP :
         	fn_initcap(ctx, args);
         	break;
         case FN_INSTR :
         	fn_instr(ctx, args);
         	break;
         case FN_INT :
         	fn_int(ctx, args);
         	break;
         case FN_ITEM_COUNT :
         	fn_item_count(ctx, args);
         	break;
         case FN_LCASE :
         	fn_lcase(ctx, args);
         	break;
         case FN_LE :
         	fn_le(ctx, args);
         	break;
         case FN_LEN :
         	fn_len(ctx, args);
         	break;
         case FN_LESS :
         	fn_less(ctx, args);
         	break;
         case FN_LN :
         	fn_ln(ctx, args);
         	break;
         case FN_LT :
         	fn_lt(ctx, args);
         	break;
         case FN_LTRIM :
         	fn_ltrim(ctx, args);
         	break;
         case FN_MAX :
         	fn_max(ctx, args);
         	break;
         case FN_MIN :
         	fn_min(ctx, args);
         	break;
         case FN_MINUTE :
         	fn_minute(ctx, args);
         	break;
         case FN_MOD :
         	fn_mod(ctx, args);
         	break;
         case FN_MONTH :
         	fn_month(ctx, args);
         	break;
         case FN_NE :
         	fn_ne(ctx, args);
         	break;
         case FN_NEG :
         	fn_neg(ctx, args);
         	break;
         case FN_NOW :
         	fn_now(ctx, args);
         	break;
         case FN_OVER :
         	fn_over(ctx, args);
         	break;
         case FN_PLAIN :
         	fn_plain(ctx, args);
         	break;
         case FN_PLUS :
         	fn_plus(ctx, args);
         	break;
         case FN_POW :
         	fn_pow(ctx, args);
         	break;
         case FN_RND :
         	fn_rnd(ctx, args);
         	break;
         case FN_RTRIM :
         	fn_rtrim(ctx, args);
         	break;
         case FN_SECOND :
         	fn_second(ctx, args);
         	break;
         case FN_SIN :
         	fn_sin(ctx, args);
         	break;
         case FN_SQRT :
         	fn_sqrt(ctx, args);
         	break;
         case FN_SUBSTR :
         	fn_substr(ctx, args);
         	break;
         case FN_TAG :
         	fn_tag(ctx, args);
         	break;
         case FN_TAN :
         	fn_tan(ctx, args);
         	break;
         case FN_TO_DATE :
         	fn_to_date(ctx, args);
         	break;
         case FN_TRIM :
         	fn_trim(ctx, args);
         	break;
         case FN_UCASE :
         	fn_ucase(ctx, args);
         	break;
         case FN_URL :
         	fn_url(ctx, args);
         	break;
         case FN_VAR :
         	fn_var(ctx, args);
         	break;
         case FN_YEAR :
         	fn_year(ctx, args);
         	break;
      	default:
      	   break;
      }
   }
   Stack_pop(args, result);
   return retVal;
}


bool Context_calc(Context ctx, String expr, Var *result){
	List   tokens = new_List();
	Stack  ops    = new_Stack();
	Stack  args   = new_Stack();
	bool   retVal = false;
	size_t i, j;
	String s;
	if(tokenize(expr, tokens)){
		if(parse(ctx, tokens, ops, args, 0)){
         if(eval(ctx, ops, args, result)){
            /* Clean up */
            delete_Stack(ops);
            delete_Stack(args);
            if(List_getLength(tokens, &j)){
               for(i = 0; i < j; i++){
                  if(List_getItem(tokens, i, (int *)&s)){
                     delete_String(s);
                  }
               }
            }
            delete_List(tokens);
            retVal  = true;
         }
		}
	}
	return retVal;
}
