#include <stdio.h>
#include <stdlib.h>

#include "bool.h"
#include "fns.h"
#include "stack.h"

bool fn_abs(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_acs(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_asn(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_atn(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_by(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_cat(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_cos(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_current(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_date_diff(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_day(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_decode(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_div(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_entity(Context ctx, Stack args){
	bool   retVal = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_insertChar(tmp, '&', 0);
   	String_appendChar(tmp, ';');
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}

	return retVal;
}


bool fn_eq(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_eval(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_exec(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_exp(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file_cdate(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file_ext(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file_mdate(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file_name_only(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file_path(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file_size(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_file_type(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_format_date(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_format_number(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_ge(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_get_item(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_get_setting(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_gt(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_hour(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_html(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_iif(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_image_colors(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_image_comments(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_image_height(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_image_width (Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_initcap (Context ctx, Stack args){
	bool   retVal = true;
	bool   gotSpace = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	char *ch = NULL;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_getChars(tmp, &ch);
   	while(*ch){
   	   if(*ch == ' '){
   	      gotSpace = true;
   	   }
   	   else{
   	      if(gotSpace){
   	         *ch = toupper(*ch);
   	         gotSpace = false;
   	      }
   	   }
   	   ch++;
   	}
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}

	return retVal;
}


bool fn_instr(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_int(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_item_count(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_lcase (Context ctx, Stack args){
	bool   retVal = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_toLower(tmp);
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}

	return retVal;
}


bool fn_le(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_len(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_less(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_ln(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_lt(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_ltrim(Context ctx, Stack args){
	bool   retVal   = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_trimLeft(tmp);
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}
	else{
	   retVal = false;
	}

	return retVal;
}


bool fn_max(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_min(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_minute(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_mod(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_month(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_ne(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_neg(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_now(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_over(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_plain(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_plus(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_pow(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_rnd(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_rtrim(Context ctx, Stack args){
	bool   retVal   = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_trimRight(tmp);
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}
	else{
	   retVal = false;
	}

	return retVal;
}


bool fn_second(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_sin(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_sqrt(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_substr(Context ctx, Stack args){
	bool retVal = true;
	Var p1;
	Var p2;
	Var p3;

	return retVal;
}


bool fn_tag(Context ctx, Stack args){
	bool   retVal = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_insertChar(tmp, '<', 0);
   	String_appendChar(tmp, '>');
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}

	return retVal;
}


bool fn_tan(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_to_date(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_trim(Context ctx, Stack args){
	bool   retVal   = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_trimAll(tmp);
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}
	else{
	   retVal = false;
	}

	return retVal;
}


bool fn_ucase(Context ctx, Stack args){
	bool   retVal   = true;
	int    argCount = 0;
	Var    v;
	Var    n;
	String st  = INVALID_STRING;
	String tmp = INVALID_STRING;
	
	Stack_pop(args, (int *)&v);
	Var_getIntValue(v, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v);
	   Var_getStringValue(v, &st);
   	tmp = new_String(NULL);
   	String_copy(tmp, st);
   	String_toUpper(tmp);
   	n = Var_set(ctx, INVALID_STRING, tmp);
   	Stack_push(args, (int)n);
   	delete_String(tmp);
	}
	else{
	   retVal = false;
	}

	return retVal;
}


bool fn_url(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}


bool fn_var(Context ctx, Stack args){
	bool   retVal   = true;
	int    argCount = 0;
	String s;
	Var    v1;
	Var    v2;

	Stack_pop(args, (int *)&v1);
	Var_getIntValue(v1, &argCount);
	if(argCount == 1){
	   Stack_pop(args, (int *)&v1);
	   Var_getStringValue(v1, &s);
	   v2 = Var_find(ctx, s);
	   Stack_push(args, v2);
	}
	else{
	   retVal = false;
	}

	return retVal;
}


bool fn_year(Context ctx, Stack args){
	bool retVal = true;

	return retVal;
}



