/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Expr.h,v 1.8 2010/08/25 09:46:20 ken Exp $
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
#include "BileObj.h"
#include "List.h"

/** Error status if expression ends prematurely */
#define EXPR_STATUSEOE 1
/** Error status if the parentheses in the expression are not balanced */
#define EXPR_STATUSPAREN 2

typedef struct tag_expr {
	/** The tokenised expression */
	List    *tokens;
	/** If True, the tokens "belong" to the expression structure and can be 
	*** freed when it is freed
	**/
	bool    freeTokens;
	/** The object in whose context the expression is to be evaluated */
	BileObject *context;
	/** Jump buffer to allow quick exit from evaluation */
	jmp_buf env;
	/** Stored status value if returning via longjmp */
	int     status;
} Expr;


Expr *new_Expr(const char *expression, BileObject *context);
Expr *new_ExprFromTokens(List *tokens, BileObject *context);
void delete_Expr(Expr *e);

char *Expr_evaluate(Expr *e);

char *evaluateString(const char *expression, BileObject *context);
char *evaluateTokens(List *tokens, BileObject *context);

#endif /* EXPR_H */
