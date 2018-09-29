static char util__[]="@(#)util.h	1.1";
#include "ctype.h"

#define ISDOUB(c)	(c=='.' || c=='-' || isdigit(c) || c=='e' || c=='+'\
			|| c=='E')
#define ISID(c)		(c=='_' || c=='.' || isalpha(c) || isdigit(c))
#define ISNUM(c)	(c=='.' || isdigit(c))

#define LOG(b,x)	(log(fabs((double)(x)))/log((double)b))
#define MAX(x,y)	(x<y ? y : x)
#define MIN(x,y)	(x>y ? y : x)

#define ABS(x)		(x>-x ? x : -x)
#define TRUE            1
#define FALSE           0
#define SUCCESS         1
#define FAIL            0
#define YES		1
#define NO		0

#define EOS             '\0'
double fabs();
