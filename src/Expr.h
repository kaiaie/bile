/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Expr.h,v 1.7 2010/08/24 21:08:42 ken Exp $
**/
/** \file Expr.h
*** \brief Expression evaluator
***
*** This is a parser for a simple expression language (i.e. it only evaluates 
*** arithmetical and logical expressions; there are no branches (though the 
*** C-style ternary conditional operator is supported) or loops. 
**/
#ifndef EXPR_H
#define EXPR_H
#include <setjmp.h>
#include "Dict.h"
#include "List.h"
#include "Vars.h"

#define EXPR_STATUSEOE 1
#define EXPR_STATUSPAREN 2

typedef struct tag_expr {
	/** The tokenised expression */
	List    *tokens;
	/** If True, the tokens "belong" to the expression structure and can be 
	*** freed when it is freed
	**/
	bool    freeTokens;
	/** The variables in whose context the expression is to be evaluated */
	Vars    *variables;
	/** Jump buffer to allow quick exit from evaluation */
	jmp_buf env;
	/** Stored status value if returning via longjmp */
	int     status;
} Expr;


Expr *new_Expr(const char *expression, Vars *variables);
Expr *new_Expr2(List *tokens, Vars *variables);
void delete_Expr(Expr *e);

char *Expr_evaluate(Expr *e);

char *evaluateExpression(const char *expression, Vars *variables);
char *evaluateTokens(List *tokens, Vars *variables);

#endif /* EXPR_H */
