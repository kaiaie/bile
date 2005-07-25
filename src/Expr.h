/* :tabSize=4:indentSize=4:folding=indent: */
#ifndef _EXPR_H
#define _EXPR_H
#include "Dict.h"
#include "List.h"
#include "Vars.h"

typedef struct _expr{
	List *tokens;
	Vars *variables;
	Dict *functions;
} Expr;


Expr *new_Expr(const char *expression, Dict *variables, Dict *functions);
void delete_Expr(Expr *e);

char *Expr_evaluate(Expr *e);

#endif /* _EXPR_H */
