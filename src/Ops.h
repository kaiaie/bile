/** \file Ops.h
*** \brief Implements the arithmetic and logical operators used by the 
*** expression language
***
*** Each operator function returns a string containing its result; it is the 
*** responsibility of the caller to free() this string once they are done with 
*** it.
***
*** \sa Expr
**/
#ifndef OPS_H
#define OPS_H

char *Op_add(char *arg1, char *arg2);
char *Op_and(char *arg1, char *arg2);
char *Op_cat(char *arg1, char *arg2);
char *Op_div(char *arg1, char *arg2);
char *Op_eq(char *arg1, char *arg2);
char *Op_ge(char *arg1, char *arg2);
char *Op_gt(char *arg1, char *arg2);
char *Op_idiv(char *arg1, char *arg2);
char *Op_le(char *arg1, char *arg2);
char *Op_lt(char *arg1, char *arg2);
char *Op_mod(char *arg1, char *arg2);
char *Op_mult(char *arg1, char *arg2);
char *Op_ne(char *arg1, char *arg2);
char *Op_neg(char *arg1);
char *Op_not(char *arg1);
char *Op_or(char *arg1, char *arg2);
char *Op_plus(char *arg1);
char *Op_pow(char *arg1, char *arg2);
char *Op_sub(char *arg1, char *arg2);
char *Op_xor(char *arg1, char *arg2);

#endif /* OPS_H */
