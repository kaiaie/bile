/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Expr.h,v 1.6 2006/06/07 21:03:20 ken Exp $
 */
#ifndef EXPR_H
#define EXPR_H
#include <setjmp.h>
#include "Dict.h"
#include "List.h"
#include "Vars.h"

#define EXPR_STATUSEOE 1
#define EXPR_STATUSPAREN 2

typedef struct _expr{
	List    *tokens;
	bool    freeTokens;
	Vars    *variables;
	jmp_buf env;
	int     status;
} Expr;


Expr *new_Expr(const char *expression, Vars *variables);
Expr *new_Expr2(List *tokens, Vars *variables);
void delete_Expr(Expr *e);

char *Expr_evaluate(Expr *e);

char *evaluateExpression(const char *expression, Vars *variables);
char *evaluateTokens(List *tokens, Vars *variables);

#endif /* EXPR_H */
