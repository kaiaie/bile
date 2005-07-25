#ifdef _BILETYPE_H
#define _BILETYPE_H
#include <time.h>			/* For struct tm				  */

/* Data types                                                 */
typedef unsigned long bvVar;

/* Encodings for BILE string variable.                        */
typedef enum{ENC_PLAIN, ENC_URL, ENC_HTML} bvEnc;

#endif /* _BILETYPE_H */
