static char s__[]="@(#)s.h	1.2";
#define MAXLABEL 80
#define MAXELEM 2501
#define DFLTCOL 5

#define NEGINFIN (-.9999999e38)
#define POSINFIN .9999999e38
#define DONEFLG (-.1e39)
#define UNDEF DONEFLG
#define DEF(x) ((x)>UNDEF)
#define doub double

#define PRINT(x,i,col)	printf("%-12g%c",(double)x,((i)%col ? '\t':'\n'))
#define PRINTP(x,i,col)	printf("(%g)%c",(double)x,((i)%col ? '\t':'\n'))
#define VALUE(x,exp)	((x=exp)>DONEFLG ? x:NEGINFIN)

#include "../../include/debug.h"
#include "../../include/errpr.h"
#include "../../include/setop.h"
#include "../../include/util.h"
#include "title.h"

extern char *nodename;
