#include <stdlib.h>

#include "bile.h"

/* Data types                                                 */
/* Types that a BILE variable can be.                         */
typedef enum{VAR_EMPTY, VAR_BOOL, VAR_INT, VAR_REAL, VAR_DATE, 
		VAR_STRING} bvType;

/* BILE variable record.                                      */
union bvVal{
   long      intVal;
   double    realVal;
   struct tm dateVal;
   char      *strVal;
};

typedef struct _bvVarRec{
   bvVar       id;
   char        *name;
   bvType      type;
   bvEnc       encoding;
   union bvVal theValue;
} bvVarRec;


bool bvGetBool(bvVar id);
long bvGetInt(bvVar id);
double bvGetReal(bvVar id);
struct tm *bvGetDate(byVar id);
char *bvGetStr(byVar id);


void bvSetBool(bvVar id, bool val);
void bvSetInt(bvVar id, long val);
void bvSetReal(bvVar id, double val);
void bvSetDate(bvVar id, struct tm *val);
void bvSetStr(bvVar id, char *val, bvEnc enc);
