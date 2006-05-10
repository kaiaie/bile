/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Expr.c,v 1.5 2006/05/10 15:01:18 ken Exp $
 * Expr - Expression language parser
 * This is a parser for a simple expression language (i.e. it only evaluates 
 * arithmetical expressions; there are no conditionals, looping constructs, 
 * etc.)
 */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Expr.h"
#include "astring.h"
#include "bool.h"
#include "Buffer.h"
#include "Func.h"
#include "Logging.h"
#include "Ops.h"
#include "memutils.h"
#include "stringext.h"
#include "tokenize.h"
#include "Type.h"

/* Parser functions */
char *bexp(Expr *e);
char *bterm(Expr *e);
char *notf(Expr *e);
char *bfact(Expr *e);
char *rel(Expr *e);
char *expr(Expr *e);
char *term(Expr *e);
char *sgnf(Expr *e);
char *fact(Expr *e);
char *expt(Expr *e);
char *func(Expr *e);

/* Warning messages */
#define EXPR_WARNEOE "Unexpected end of expression"


Expr *new_Expr(const char *expression, Vars *variables){
	Expr *result = NULL;
	
	result = (Expr *)mu_malloc(sizeof(Expr));
	result->tokens    = tokenize(expression);
	result->variables = variables;
	return result;
} /* new_Expr */


/* FIXME */
Expr *new_Expr2(List *tokens, Vars *variables){
	Expr *result = NULL;
	
	result = (Expr *)mu_malloc(sizeof(Expr));
	result->tokens    = tokens;
	result->variables = variables;
	return result;
} /* new_Expr2 */



void delete_Expr(Expr *e){
	if(e != NULL){
		if(e->tokens != NULL){
			delete_List(e->tokens, true);
		}
		mu_free(e);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
} /* delete_Expr */


char *Expr_evaluate(Expr *e){
	List *tokens = NULL;
	if(e != NULL){
		tokens = e->tokens;
		if(List_length(tokens) > 0){
			List_moveFirst(tokens);
			return bexp(e);
		}
		else{
			Logging_warnf("%s(): Empty expression!", __FUNCTION__);
			return astrcpy("");
		}
	}
	else{
		Logging_warnNullArg(__FUNCTION__);
		return NULL;
	}
} /* Expr_evaluate */


char *bexp(Expr *e){
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = bterm(e);
	while(List_moveNext(tokens)){
		op = (char *)List_current(tokens);
		if(!strequals(op, "or") && !strequals(op, "xor")){
			List_movePrevious(tokens);
			break;
		}
		if(List_moveNext(tokens)){
			arg2 = bterm(e);
			if(strequals(op, "or")){
				tmp = Op_or(arg1, arg2);
			}
			else if(strequals(op, "xor")){
				tmp = Op_xor(arg1, arg2);
			}
			mu_free(arg1);
			mu_free(arg2);
			arg1 = tmp;
		}
		else{
			Logging_warn(EXPR_WARNEOE);
			return astrcpy("false");
		}
	}
	return arg1;
} /* bexp */


char *bterm(Expr *e){
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = notf(e);
	while(List_moveNext(tokens)){
		op = (char *)List_current(tokens);
		if(!strequals(op, "and")){
			List_movePrevious(tokens);
			break;
		}
		else{
			if(List_moveNext(tokens)){
				arg2 = notf(e);
				tmp  = Op_and(arg1, arg2);
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else{
				Logging_warn(EXPR_WARNEOE);
				return astrcpy("false");
			}
		}
	}
	return arg1;
} /* bterm */


char *notf(Expr *e){
	List *tokens = e->tokens;
	char *tmp    = NULL;
	char *retVal = NULL;
	
	if(strequals((char *)List_current(tokens), "not")){
		if(List_moveNext(tokens)){
			tmp = bfact(e);
			retVal = Op_not(tmp);
			mu_free(tmp);
		}
		else{
			Logging_warn(EXPR_WARNEOE);
			retVal = astrcpy("false");
		}
	}
	else{
		retVal = bfact(e);
	}
	return retVal;
} /* notf */


char *bfact(Expr *e){
	List *tokens = e->tokens;
	char *retVal = NULL;
	
	if(strequals((char *)List_current(tokens), "true")){
		retVal = astrcpy("true");
	}
	else if(strequals((char *)List_current(tokens), "false")){
		retVal = astrcpy("false");
	}
	else{
		retVal = rel(e);
	}
	return retVal;
} /* bfact */


char *rel(Expr *e){
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = expr(e);
	if(List_moveNext(tokens)){
		op = (char *)List_current(tokens);
		if(strequals(op, "eq") ||
				strequals(op, "ne") ||
				strequals(op, "lt") ||
				strequals(op, "gt") ||
				strequals(op, "le") ||
				strequals(op, "ge")){
			if(List_moveNext(tokens)){
				arg2 = expr(e);
				if(strequals(op, "eq")){
					tmp = Op_eq(arg1, arg2);
				}
				else if(strequals(op, "ne")){
					tmp = Op_ne(arg1, arg2);
				}
				else if(strequals(op, "lt")){
					tmp = Op_lt(arg1, arg2);
				}
				else if(strequals(op, "gt")){
					tmp = Op_gt(arg1, arg2);
				}
				else if(strequals(op, "le")){
					tmp = Op_le(arg1, arg2);
				}
				else if(strequals(op, "ge")){
					tmp = Op_ge(arg1, arg2);
				}
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else{
				Logging_warn(EXPR_WARNEOE);
			}
		}
		else{
			List_movePrevious(tokens);
		}
	}
	return arg1;
} /* rel */


char *expr(Expr *e){
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = term(e);
	while(List_moveNext(tokens)){
		op = (char *)List_current(tokens);
		if(strequals(op, "+") || 
				strequals(op, "-") ||
				strequals(op, ".")){
			if(List_moveNext(tokens)){
				arg2 = term(e);
				if(strequals(op, "+")){
					tmp = Op_add(arg1, arg2);
				}
				else if(strequals(op, "-")){
					tmp = Op_sub(arg1, arg2);
				}
				else if(strequals(op, ".")){
					tmp = Op_cat(arg1, arg2);
				}
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else{
				Logging_warn(EXPR_WARNEOE);
			}
		}
		else{
			List_movePrevious(tokens);
			break;
		}
	}
	return arg1;
} /* expr */


char *term(Expr *e){
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = sgnf(e);
	while(List_moveNext(tokens)){
		op = (char *)List_current(tokens);
		if(strequals(op, "*") || 
				strequals(op, "/") || 
				strequals(op, "mod") || 
				strequals(op, "div")){
			if(List_moveNext(tokens)){
				arg2 = sgnf(e);
				if(strequals(op, "*")){
					tmp = Op_mult(arg1, arg2);
				}
				else if(strequals(op, "/")){
					tmp = Op_div(arg1, arg2);
				}
				else if(strequals(op, "mod")){
					tmp = Op_mod(arg1, arg2);
				}
				else if(strequals(op, "div")){
					tmp = Op_idiv(arg1, arg2);
				}
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else{
				Logging_warn(EXPR_WARNEOE);
			}
		}
		else{
			List_movePrevious(tokens);
			break;
		}
	}
	return arg1;
} /* term */


char *sgnf(Expr *e){
	List *tokens = e->tokens;
	char *sign   = NULL;
	char *tmp    = NULL;
	char *retVal = NULL;
	
	sign = (char *)List_current(tokens);
	if(strequals(sign, "+") || strequals(sign, "-")){
		if(List_moveNext(tokens)){
			tmp = fact(e);
			if(strequals(sign, "+")){
				retVal = Op_plus(tmp);
			}
			else if(strequals(sign, "-")){
				retVal = Op_neg(tmp);
			}
			mu_free(tmp);
		}
		else{
			Logging_warn(EXPR_WARNEOE);
			retVal = astrcpy("false");
		}
	}
	else{
		retVal = fact(e);
	}
	return retVal;
} /* sgnf */


char *fact(Expr *e){
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = expt(e);
	if(List_moveNext(tokens)){
		op = (char *)List_current(tokens);
		if(strequals(op, "^")){
			if(List_moveNext(tokens)){
				arg2 = expt(e);
				tmp  = Op_pow(arg1, arg2);
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else{
				Logging_warn(EXPR_WARNEOE);
			}
		}
		else{
			List_movePrevious(tokens);
		}
	}
	return arg1;
} /* fact */


char *expt(Expr *e){
	List *tokens = e->tokens;
	char *curr   = NULL;
	char *retVal = NULL;
	
	curr = (char *)List_current(tokens);
	if(curr[0] == '$'){
		/* Variable */
		retVal = astrcpy(Vars_get(e->variables, &curr[1]));
	}
	else if(curr[0] == '`'){
		/* String literal */
		retVal = astrunquote(curr);
	}
	else if(strequals(curr, "(")){
		/* Parenthetical expression */
		if(List_moveNext(tokens)){
			retVal = bexp(e);
			if(List_moveNext(tokens)){
				curr = (char *)List_current(tokens);
				if(!strequals(curr, ")")){
					Logging_warnf("%s(): Expected \")\", got \"%s\"",
							__FUNCTION__, 
							(char *)List_current(tokens));
				}
			}
			else{
				Logging_warn(EXPR_WARNEOE);
			}
		}
		else{
			Logging_warn(EXPR_WARNEOE);
		}
	}
	else if(curr[strlen(curr) - 1] == '('){
		/* Function call */
		retVal = func(e);
	}
	else{
		retVal = astrcpy(curr);
	}
	return retVal;
} /* expt */


char *func(Expr *e){
	List   *tokens   = e->tokens;
	char   *funcName = NULL;
	char   *curr     = NULL;
	List   *args     = new_List();
	size_t argLength = 0;
	char   **argList = NULL;
	char   *retVal   = NULL;
	bool   first     = true;
	char *(*func)(int argc, char *argv[]) = NULL;
	
	funcName = (char *)List_current(tokens);
	while(List_moveNext(tokens)){
		if(first){
			/* Special case: function with no args */
			first = false;
			if(strequals((char *)List_current(tokens), ")")) break;
		}
		List_append(args, expr(e));
		if(List_moveNext(tokens)){
			curr = (char *)List_current(tokens);
			if(strequals(curr, ",")){
				if(!List_moveNext(tokens)){
					Logging_warn(EXPR_WARNEOE);
				}
				else{
					List_movePrevious(tokens);
				}
			}
			else if(strequals(curr, ")")){
				break;
			}
		}
	}
	if(Dict_exists(getFunctionList(), funcName)){
		func = Dict_get(getFunctionList(), funcName);
		argLength = List_length(args);
		argList = (char **)List_toArray(args, false);
		retVal = (*func)(argLength, argList);
	}
	else{
		Logging_warnf("%s(): Call to undefined function: \"%s\"",
				__FUNCTION__, 
				funcName);
		retVal = astrcpy("");
	}
	mu_free(argList);
	delete_List(args, true);
	return retVal;
} /* func */
