#ifndef _VAR_H
#define _VAR_H
#include <time.h>
#include "str.h"

#define INVALID_VAR 0
#define INVALID_CONTEXT 0

#define VF_PLAIN 0
#define VF_HTML 1
#define VF_URL 2
#define VF_CONST 4

typedef int Context;
typedef int Var;

Var bileTrue;
Var bileFalse;
Var bilePi;
Var bileErr;

Context new_Context(Context parentContext);
void delete_Context(Context context);

bool Context_getParent(Context context, Context *parent);

Var Var_set(Context context, String name, String value);
void delete_Var(Var v);

Var Var_find(Context context, String name);
bool Var_setFlags(Var v, unsigned char flags);

bool Var_getFlags(Var v, unsigned char *flags);
bool Var_getStringValue(Var v, String *value);
bool Var_getIntValue(Var v, int *value);
bool Var_getDoubleValue(Var v, double *value);
bool Var_getBoolValue(Var v, bool *value);
bool Var_getDateValue(Var v, time_t *value);

#endif /* _VAR_H */
