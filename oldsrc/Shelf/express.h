#ifndef _EXPRESS_H
#define _EXPRESS_H
#include <stddef.h>
#include "bool.h"
#include "str.h"
#include "var.h"

bool Context_calc(Context ctx, String expr, Var *result);

#endif /* _EXPRESS_H */
