/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Ops.c,v 1.2 2006/01/08 18:02:54 ken Exp $
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "astring.h"
#include "bool.h"
#include "Logging.h"
#include "Ops.h"
#include "Type.h"


char *Op_add(char *arg1, char *arg2){
	/* Addition operator */
	long   iarg1, iarg2;
	double darg1, darg2;
	char   *result = NULL;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			result = asprintf("%ld", (iarg1 + iarg2));
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = asprintf("%f", (darg1 + darg2));
		}
	}
	else{
		Logging_warn("%s(): Non-numeric argument.", __FUNCTION__);
		result = Op_cat(arg1, arg2);
	}
	return result;	
}


char *Op_and(char *arg1, char *arg2){
	/* Logical AND */
	char *result = NULL;
	bool barg1, barg2;
	
	barg1 = Type_toBool(arg1);
	barg2 = Type_toBool(arg2);
	if(arg1){
		result = astrcpy(arg2);
	}
	else{
		result = astrcpy("false");
	}
	return result;
}


char *Op_cat(char *arg1, char *arg2){
	/* String concatenation */
	return astrcat(arg1, arg2);
}


char *Op_div(char *arg1, char *arg2){
	/* Floating-point division */
	long   iarg1, iarg2;
	double darg1, darg2;
	char   *result = NULL;
	bool   nonNumeric   = false;
	bool   divideByZero = false;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			if(iarg2 == 0){
				divideByZero = true;
			}
			else{
				result = asprintf("%ld", (iarg1 / iarg2));
			}
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			if(darg2 == 0.0){
				divideByZero = true;
			}
			else{
				result = asprintf("%f", (darg1 / darg2));
			}
		}
	}
	else{
		nonNumeric = true;
	}
	if(divideByZero || nonNumeric){
		if(divideByZero){
			Logging_warn("%s(): Divide by zero", __FUNCTION__);
		}
		if(nonNumeric){
			Logging_warn("%s(): Non-numeric argument", __FUNCTION__);
		}
		result = astrcpy("0");
	}
	return result;
}


char *Op_eq(char *arg1, char *arg2){
	/* "=" relational operator */
	long   iarg1, iarg2;
	double darg1, darg2;
	bool   result = false;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			result = (iarg1 == iarg2);
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = (darg1 == darg2);
		}
	}
	else{
		result = (strcmp(arg1, arg2) == 0);
	}
	if(result){
		return astrcpy("true");
	}
	else{
		return astrcpy("false");
	}
}


char *Op_ge(char *arg1, char *arg2){
	/* ">=" relational operator */
	char *tmp    = Op_lt(arg1, arg2);
	char *result = Op_not(tmp);
	free(tmp);
	return result;
}


char *Op_gt(char *arg1, char *arg2){
	/* ">" relational operator */
	long   iarg1, iarg2;
	double darg1, darg2;
	bool   result = false;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			result = (iarg1 > iarg2);
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = (darg1 > darg2);
		}
	}
	else{
		result = (strcmp(arg1, arg2) > 0);
	}
	if(result){
		return astrcpy("true");
	}
	else{
		return astrcpy("false");
	}
}


char *Op_idiv(char *arg1, char *arg2){
	/* Integer division */
	char *result = Op_div(arg1, arg2);
	char *tmp = NULL;
	
	if((tmp = strchr(result, '.')) != NULL){
		*tmp = '\0';
	}
	return result;
}


char *Op_le(char *arg1, char *arg2){
	/* "<=" relational operator */
	char *tmp    = Op_gt(arg1, arg2);
	char *result = Op_not(tmp);
	free(tmp);
	return result;
}


char *Op_lt(char *arg1, char *arg2){
	/* "<" relational operator */
	long   iarg1, iarg2;
	double darg1, darg2;
	bool   result = false;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			result = (iarg1 < iarg2);
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = (darg1 < darg2);
		}
	}
	else{
		result = (strcmp(arg1, arg2) < 0);
	}
	if(result){
		return astrcpy("true");
	}
	else{
		return astrcpy("false");
	}
}


char *Op_mod(char *arg1, char *arg2){
	/* Modulus operator */
	long   iarg1, iarg2;
	double darg1, darg2;
	char   *result = NULL;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			result = asprintf("%ld", (iarg1 % iarg2));
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = asprintf("%f", fmod(darg1, darg2));
		}
	}
	else{
		Logging_warn("%s(): Non-numeric argument", __FUNCTION__);
		result = astrcpy("0");
	}
	return result;
}


char *Op_mult(char *arg1, char *arg2){
	/* Multiplication */
	long   iarg1, iarg2;
	double darg1, darg2;
	char   *result = NULL;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			result = asprintf("%ld", (iarg1 * iarg2));
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = asprintf("%f", (darg1 * darg2));
		}
	}
	else{
		Logging_warn("%s(): Non-numeric argument", __FUNCTION__);
		result = astrcpy("0");
	}
	return result;	
}


char *Op_ne(char *arg1, char *arg2){
	/* Logical "<>" operator */
	char *tmp    = Op_eq(arg1, arg2);
	char *result = Op_not(tmp);
	free(tmp);
	return result;
}


char *Op_neg(char *arg1){
	/* Unary minus operator */
	char *result = NULL;
	long   iarg1;
	double darg1;
	
	if(Type_isNumeric(arg1)){
		if(Type_isLong(arg1)){
			iarg1 = Type_toLong(arg1) * -1;			
			result = asprintf("%ld", iarg1);
		}
		else{
			darg1 = Type_toDouble(arg1) * -1.0;
			result = asprintf("%f", darg1);
		}
	}
	else{
		Logging_warn("%s(): Non-numeric argument", __FUNCTION__);
		result = astrcpy("0");
	}
	return result;
}


char *Op_not(char *arg1){
	/* Logical NOT operator */
	char *result = NULL;
	bool tmp = false;
	
	tmp = Type_toBool(arg1);
	if(tmp){
		result = astrcpy("false");
	}
	else{
		result = astrcpy("true");
	}
	return result;
}


char *Op_or(char *arg1, char *arg2){
	/* Logical (inclusive) OR */
	bool barg1, barg2;
	char *result = NULL;
	
	barg1 = Type_toBool(arg1);
	barg2 = Type_toBool(arg2);
	
	if(barg1){
		result = astrcpy(arg1);
	}
	else if(barg2){
		result = astrcpy(arg2);
	}
	else{
		result = astrcpy("true");
	}
	return result;
}


char *Op_plus(char *arg1){
	/* Unary plus operator */
	char *result = NULL;
	
	if(Type_isNumeric(arg1)){
		if(Type_isLong(arg1)){
			result = asprintf("%ld", Type_toLong(arg1));
		}
		else{
			result = asprintf("%f", Type_toDouble(arg1));
		}
	}
	else{
		Logging_warn("%s(): Non-numeric argument", __FUNCTION__);
		result = astrcpy("0");
	}
	return result;
}


char *Op_pow(char *arg1, char *arg2){
	/* Exponention operator */
	long   iarg1, iarg2;
	long   tmp;
	long   ii;
	double darg1, darg2;
	char   *result = NULL;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			if(iarg2 == 0){
				tmp = 1;
			}
			else{
				tmp = 1;
				for(ii = 0; ii < iarg2; ++ii){
					tmp *= iarg1;
				}
			}
			result = asprintf("%ld", tmp);
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = asprintf("%f", pow(darg1, darg2));
		}
	}
	else{
		Logging_warn("%s(): Non-numeric argument", __FUNCTION__);
		result = astrcpy("0");
	}
	return result;	
}


char *Op_sub(char *arg1, char *arg2){
	/* Subtraction operator */
	long   iarg1, iarg2;
	double darg1, darg2;
	char   *result = NULL;
	
	if(Type_isNumeric(arg1) && Type_isNumeric(arg2)){
		if(Type_isLong(arg1) && Type_isLong(arg2)){
			iarg1 = Type_toLong(arg1);
			iarg2 = Type_toLong(arg2);
			result = asprintf("%ld", (iarg1 - iarg2));
		}
		else{
			darg1 = Type_toDouble(arg1);
			darg2 = Type_toDouble(arg2);
			result = asprintf("%f", (darg1 - darg2));
		}
	}
	else{
		Logging_warn("%s(): Non-numeric argument", __FUNCTION__);
		result = astrcpy("0");
	}
	return result;	
}


char *Op_xor(char *arg1, char *arg2){
	/* Logical exclusive OR */
	bool barg1, barg2;
	char *result = NULL;
	
	barg1 = Type_toBool(arg1);
	barg2 = Type_toBool(arg2);
	
	if(barg1 && barg2){
		result = astrcpy("false");
	}
	else{
		result = Op_or(arg1, arg2);
	}
	return result;
}


