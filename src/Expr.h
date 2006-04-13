/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Expr.h,v 1.4 2006/04/13 00:01:51 ken Exp $
 */
#ifndef EXPR_H
#define EXPR_H
#include "Dict.h"
#include "List.h"
#include "Vars.h"

typedef struct _expr{
	List *tokens;
	Vars *variables;
} Expr;


Expr *new_Expr(const char *expression, Vars *variables);
Expr *new_Expr2(List *tokens, Vars *variables);
void delete_Expr(Expr *e);

char *Expr_evaluate(Expr *e);

#endif /* EXPR_H */
