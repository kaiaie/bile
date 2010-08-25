/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Expr.c,v 1.11 2010/08/25 10:02:41 ken Exp $
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
char *tern(Expr *e);
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


/** Allocates and initialises a new Expr structure for the supplied expression */
Expr *new_Expr(const char *expression, Vars *variables){
	Expr *result = NULL;
	
	result = (Expr *)mu_malloc(sizeof(Expr));
	result->tokens     = tokenize(expression);
	result->freeTokens = true;
	result->variables  = variables;
	return result;
} /* new_Expr */


/** Allocates and initialises a new Expr structure for the pre-tokenised 
*** expression 
***
*** \note Needs a better name!
**/
Expr *new_Expr2(List *tokens, Vars *variables){
	Expr *result = NULL;
	
	result = (Expr *)mu_malloc(sizeof(Expr));
	result->tokens     = tokens;
	result->freeTokens = false;
	result->variables  = variables;
	return result;
} /* new_Expr2 */


/** Deletes and frees the Expr structure */
void delete_Expr(Expr *e){
	if(e != NULL){
		if(e->tokens != NULL && e->freeTokens){
			/* Only delete the token list if it was allocated by new_Expr */
			delete_List(e->tokens, true);
		}
		mu_free(e);
	}
	else
		Logging_warnNullArg(__FUNCTION__);
} /* delete_Expr */


/** Evaluates the expression structure and returns the result as a string
***
*** \note It is the caller's responsibility to free the result string.
**/
char *Expr_evaluate(Expr *e){
	List *tokens = NULL;
	int  status;
	char *result = NULL;
	if(e != NULL){
		tokens = e->tokens;
		if(List_length(tokens) > 0){
			List_moveFirst(tokens);
			if((status = setjmp(e->env)) == 0){
				result =  tern(e);
			}
			else{
				e->status = status;
				if(status == EXPR_STATUSEOE)
					Logging_warn("Unexpected end of expression");
				else if(status == EXPR_STATUSPAREN)
					Logging_warn("Missing closing parenthesis");
				result = astrcpy("");
			}
			return result;
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


/** Evaluates the specified expression using the specified 
*** variables.
***
*** This is a convenience function. It can be used when there is no need to 
*** reuse an Expr structure.
**/
char *evaluateExpression(const char *expression, Vars *variables){
	Expr *e      = new_Expr(expression, variables);
	char *result = Expr_evaluate(e);
	delete_Expr(e);
	return result;
} /* evaluateExpression */


/** Evaluates the pre-tokenised expression using the specified 
*** variables.
***
*** This is a convenience function. It can be used when there is no need to 
*** reuse an Expr structure.
**/
char *evaluateTokens(List *tokens, Vars *variables){
	Expr *e      = new_Expr2(tokens, variables);
	char *result = Expr_evaluate(e);
	delete_Expr(e);
	return result;
} /* evaluateTokens */


char *tern(Expr *e) {
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *arg2   = NULL;
	char *arg3   = NULL;
	char *curr   = NULL;
	char *result = NULL;
	bool test;
	
	arg1 = bexp(e);
	if (List_moveNext(tokens)) {
		test = Type_toBool(arg1);
		mu_free(arg1);
		curr = List_currentString(tokens);
		if (strxequals(curr, "?")) {
			if (List_moveNext(tokens)) {
				arg2 = bexp(e);
				if (List_moveNext(tokens)) {
					curr = List_currentString(tokens);
					if (strxequals(curr, ":")) {
						if (List_moveNext(tokens)) {
							arg3 = bexp(e);
							if (test) {
								result = arg2;
								mu_free(arg3);
							}
							else {
								result = arg3;
								mu_free(arg2);
							}
						}
						else {
							longjmp(e->env, EXPR_STATUSEOE);
						}
					}
					else {
						longjmp(e->env, EXPR_STATUSEOE);
					}
				}
				else {
					longjmp(e->env, EXPR_STATUSEOE);
				}
			}
			else {
				longjmp(e->env, EXPR_STATUSEOE);
			}
		}
		else {
			longjmp(e->env, EXPR_STATUSEOE);
		}
	}
	else {
		result = arg1;
	}
	return result;
}


char *bexp(Expr *e) {
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = bterm(e);
	while (List_moveNext(tokens)) {
		op = List_currentString(tokens);
		if (!strxequals(op, "or") && !strxequals(op, "xor")){
			List_movePrevious(tokens);
			break;
		}
		if (List_moveNext(tokens)) {
			arg2 = bterm(e);
			if (strxequals(op, "or")) {
				tmp = Op_or(arg1, arg2);
			}
			else if (strxequals(op, "xor")) {
				tmp = Op_xor(arg1, arg2);
			}
			mu_free(arg1);
			mu_free(arg2);
			arg1 = tmp;
		}
		else {
			longjmp(e->env, EXPR_STATUSEOE);
		}
	}
	return arg1;
} /* bexp */


char *bterm(Expr *e) {
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = notf(e);
	while (List_moveNext(tokens)) {
		op = List_currentString(tokens);
		if (!strxequals(op, "and")) {
			List_movePrevious(tokens);
			break;
		}
		else {
			if (List_moveNext(tokens)) {
				arg2 = notf(e);
				tmp  = Op_and(arg1, arg2);
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else {
				longjmp(e->env, EXPR_STATUSEOE);
			}
		}
	}
	return arg1;
} /* bterm */


char *notf(Expr *e) {
	List *tokens = e->tokens;
	char *tmp    = NULL;
	char *retVal = NULL;
	
	if (strxequals(List_currentString(tokens), "not")) {
		if (List_moveNext(tokens)) {
			tmp = bfact(e);
			retVal = Op_not(tmp);
			mu_free(tmp);
		}
		else {
			longjmp(e->env, EXPR_STATUSEOE);
		}
	}
	else {
		retVal = bfact(e);
	}
	return retVal;
} /* notf */


char *bfact(Expr *e) {
	List *tokens = e->tokens;
	char *retVal = NULL;
	
	if (strxequals(List_currentString(tokens), "true")) {
		retVal = astrcpy("true");
	}
	else if (strxequals(List_currentString(tokens), "false")) {
		retVal = astrcpy("false");
	}
	else {
		retVal = rel(e);
	}
	return retVal;
} /* bfact */


char *rel(Expr *e) {
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = expr(e);
	if (List_moveNext(tokens)){
		op = List_currentString(tokens);
		if (strxequals(op, "eq") ||
			strxequals(op, "ne") ||
			strxequals(op, "lt") ||
			strxequals(op, "gt") ||
			strxequals(op, "le") ||
			strxequals(op, "ge")
		) {
			if (List_moveNext(tokens)) {
				arg2 = expr(e);
				if (strxequals(op, "eq")) {
					tmp = Op_eq(arg1, arg2);
				}
				else if (strxequals(op, "ne")) {
					tmp = Op_ne(arg1, arg2);
				}
				else if (strxequals(op, "lt")) {
					tmp = Op_lt(arg1, arg2);
				}
				else if (strxequals(op, "gt")) {
					tmp = Op_gt(arg1, arg2);
				}
				else if (strxequals(op, "le")) {
					tmp = Op_le(arg1, arg2);
				}
				else if (strxequals(op, "ge")) {
					tmp = Op_ge(arg1, arg2);
				}
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else {
				longjmp(e->env, EXPR_STATUSEOE);
			}
		}
		else {
			List_movePrevious(tokens);
		}
	}
	return arg1;
} /* rel */


char *expr(Expr *e)  {
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = term(e);
	while (List_moveNext(tokens)) {
		op = List_currentString(tokens);
		if (strxequals(op, "+") || 
			strxequals(op, "-") ||
			strxequals(op, ".")
		) {
			if (List_moveNext(tokens)) {
				arg2 = term(e);
				if (strxequals(op, "+")) {
					tmp = Op_add(arg1, arg2);
				}
				else if (strxequals(op, "-")) {
					tmp = Op_sub(arg1, arg2);
				}
				else if (strxequals(op, ".")) {
					tmp = Op_cat(arg1, arg2);
				}
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else {
				longjmp(e->env, EXPR_STATUSEOE);
			}
		}
		else {
			List_movePrevious(tokens);
			break;
		}
	}
	return arg1;
} /* expr */


char *term(Expr *e) {
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = sgnf(e);
	while (List_moveNext(tokens)) {
		op = List_currentString(tokens);
		if (strxequals(op, "*") || 
			strxequals(op, "/") || 
			strxequals(op, "mod") || 
			strxequals(op, "div")
		) {
			if (List_moveNext(tokens)) {
				arg2 = sgnf(e);
				if (strxequals(op, "*")) {
					tmp = Op_mult(arg1, arg2);
				}
				else if (strxequals (op, "/")) {
					tmp = Op_div(arg1, arg2);
				}
				else if (strxequals(op, "mod")) {
					tmp = Op_mod(arg1, arg2);
				}
				else if (strxequals(op, "div")) {
					tmp = Op_idiv(arg1, arg2);
				}
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else {
				longjmp(e->env, EXPR_STATUSEOE);
			}
		}
		else {
			List_movePrevious(tokens);
			break;
		}
	}
	return arg1;
} /* term */


char *sgnf(Expr *e) {
	List *tokens = e->tokens;
	char *sign   = NULL;
	char *tmp    = NULL;
	char *retVal = NULL;
	
	sign = List_currentString(tokens);
	if (strxequals(sign, "+") || strxequals(sign, "-")) {
		if (List_moveNext(tokens)) {
			tmp = fact(e);
			if(strxequals(sign, "+")){
				retVal = Op_plus(tmp);
			}
			else if (strxequals(sign, "-")) {
				retVal = Op_neg(tmp);
			}
			mu_free(tmp);
		}
		else {
			longjmp(e->env, EXPR_STATUSEOE);
		}
	}
	else{
		retVal = fact(e);
	}
	return retVal;
} /* sgnf */


char *fact(Expr *e) {
	List *tokens = e->tokens;
	char *arg1   = NULL;
	char *op     = NULL;
	char *arg2   = NULL;
	char *tmp    = NULL;
	
	arg1 = expt(e);
	if (List_moveNext(tokens)) {
		op = List_currentString(tokens);
		if (strxequals(op, "^")) {
			if (List_moveNext(tokens)) {
				arg2 = expt(e);
				tmp  = Op_pow(arg1, arg2);
				mu_free(arg1);
				mu_free(arg2);
				arg1 = tmp;
			}
			else {
				longjmp(e->env, EXPR_STATUSEOE);
			}
		}
		else {
			List_movePrevious(tokens);
		}
	}
	return arg1;
} /* fact */


char *expt(Expr *e) {
	List *tokens = e->tokens;
	char *curr   = NULL;
	char *retVal = NULL;
	
	curr = List_currentString(tokens);
	if (curr[0] == '$' && curr[strlen(curr) - 1] != '(') {
		if (curr[1] != '$') {
			/* Variable */
			retVal = astrcpy(Vars_get(e->variables, &curr[1]));
		}
		else{
			/* Variable-variable */
			retVal = astrcpy(Vars_get(e->variables, 
				Vars_get(e->variables, &curr[2])));
		}
	}
	else if (strchr("`'\"", curr[0]) != NULL) {
		/* String literal */
		retVal = astrunquote(curr);
	}
	else if (strxequals(curr, "(")) {
		/* Parenthetical expression */
		if (List_moveNext(tokens)) {
			retVal = bexp(e);
			if (List_moveNext(tokens)) {
				curr = List_currentString(tokens);
				if (!strxequals(curr, ")")) {
					Logging_warnf("%s(): Expected \")\", got \"%s\"",
							__FUNCTION__, 
							List_currentString(tokens));
				}
			}
			else {
				longjmp(e->env, EXPR_STATUSEOE);
			}
		}
		else {
			longjmp(e->env, EXPR_STATUSEOE);
		}
	}
	else if (curr[strlen(curr) - 1] == '(') {
		/* Function call */
		retVal = func(e);
	}
	else {
		retVal = astrcpy(curr);
	}
	return retVal;
} /* expt */


char *func(Expr *e) {
	List *tokens   = e->tokens;
	char *funcName = NULL;
	char *curr     = NULL;
	List *args     = new_List();
	bool first     = true;
	char *tmp      = NULL;
	char *retVal   = NULL;
	char *(*func)(Vars *v, List *args) = NULL;
	
	curr = List_currentString(tokens);
	if (curr[0] == '$') {
		/* Variable function */
		tmp = astrmid(curr, 1, strlen(funcName) - 2);
		funcName = astrcat(Vars_get(e->variables, tmp), "(");
		mu_free(tmp);
	}
	else {
		funcName = astrcpy(curr);
	}
	while (List_moveNext(tokens)) {
		if(first){
			/* Special case: function with no args */
			first = false;
			if(strxequals(List_currentString(tokens), ")")) break;
		}
		List_append(args, expr(e));
		if (List_moveNext(tokens)) {
			curr = List_currentString(tokens);
			if (strxequals(curr, ",")) {
				if (!List_moveNext(tokens)) {
					longjmp(e->env, EXPR_STATUSEOE);
				}
				else {
					List_movePrevious(tokens);
				}
			}
			else if (strxequals(curr, ")")) {
				break;
			}
		}
	}
	if (Dict_exists(getFunctionList(), funcName)) {
		func = Dict_get(getFunctionList(), funcName);
		retVal = (*func)(e->variables, args);
	}
	else {
		Logging_warnf("%s(): Call to undefined function: \"%s\"",
				__FUNCTION__, 
				funcName);
		retVal = astrcpy("");
	}
	delete_List(args, true);
	mu_free(funcName);
	return retVal;
} /* func */

