/* :tabSize=4:indentSize=4:folding=indent:
** $Id: tokenize.h,v 1.4 2010/08/31 15:11:58 ken Exp $
**/

/** \file tokenize.h
*** \brief Implements the tokeniser for the BILE expression language
*** \sa Expr
**/

#ifndef TOKENIZE_H
#define TOKENIZE_H
#include "List.h"

List *tokenize(const char *input);

#endif /* TOKENIZE_H */
