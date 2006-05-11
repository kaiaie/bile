/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Expr.h,v 1.5 2006/05/11 10:20:42 ken Exp $
 */
#ifndef EXPR_H
#define EXPR_H
#include "Dict.h"
#include "List.h"
#include "Vars.h"

typedef struct _expr{
	List *tokens;
	bool freeTokens;
	Vars *variables;
} Expr;


Expr *new_Expr(const char *expression, Vars *variables);
Expr *new_Expr2(List *tokens, Vars *variables);
void delete_Expr(Expr *e);

char *Expr_evaluate(Expr *e);

char *evaluateExpression(const char *expression, Vars *variables);
char *evaluateTokens(List *tokens, Vars *variables);

#endif /* EXPR_H */
