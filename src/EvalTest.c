#include <stdlib.h>
#include "Buffer.h"
#include "Dict.h"
#include "Expr.h"
#include "Func.h"
#include "List.h"
#include "Logging.h"
#include "Ops.h"
#include "Type.h"
#include "Vars.h"

/* Variables */
static Vars *variables = NULL;

int main(int argc, char *argv[]){
	/* Define a default expression so I can test it without GDB screwing about
	 * with escape characters...
	 */
	char   defaultExpression[] = "substr(`This is a test`, 1, 4)";
	char   *expression = NULL;
	Buffer *b      = new_Buffer(256);
	char   *result = NULL;
	int    ii;
	Expr   *e = NULL;
	/* Variable "scopes" */
	Vars   *globals = NULL;
	Vars   *locals  = NULL;
	
	Logging_setup(argv[0], LOG_TOSTDERR | LOG_TOFILE | LOG_LEVELTRACE, "EvalTest.log");
	
	/* Add a few variables */
	globals = new_Vars(NULL);
	locals  = new_Vars(globals);
	Vars_let(globals, "pi", "3.1415", VAR_CONST | VAR_NOSHADOW);
	Vars_let(locals, "x", "1234", VAR_STD);
	Vars_let(locals, "t", "This is a test", VAR_STD);
	
	/* Build expression off command line if supplied */
	if(argc > 1){
		for(ii = 1; ii < argc; ++ii){
			Buffer_appendString(b, argv[ii]);
			Buffer_appendString(b, " ");
		}
		expression = b->data;
	}
	else{
		expression = defaultExpression;
	}
	Logging_debugf("Input string: \"%s\"", expression);
	e = new_Expr(expression, variables);
	result = Expr_evaluate(e);
	if(result != NULL){
		Logging_infof("Expression result: \"%s\"", result);
	}
	else{
		Logging_warn("Expression result was NULL");
	}
	delete_Vars(globals);
	delete_Vars(locals);
	delete_Expr(e);
	delete_Buffer(b);
	exit(EXIT_SUCCESS);
}
