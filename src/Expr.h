/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Expr.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 */
#ifndef EXPR_H
#define EXPR_H
#include "Dict.h"
#include "List.h"
#include "Vars.h"

typedef struct _expr{
	List *tokens;
	Vars *variables;
	Dict *functions;
} Expr;


Expr *new_Expr(const char *expression, Vars *variables, Dict *functions);
Expr *new_Expr2(List *tokens, Vars *variables, Dict *functions);
void delete_Expr(Expr *e);

char *Expr_evaluate(Expr *e);

#endif /* EXPR_H */
