/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/s.h	1.3"
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

#include "debug.h"
#include "errpr.h"
#include "setop.h"
#include "util.h"
#include "title.h"

extern char *nodename;
