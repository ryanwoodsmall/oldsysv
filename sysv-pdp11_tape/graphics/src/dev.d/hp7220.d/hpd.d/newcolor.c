static char SCCSID[]="@(#)newcolor.c	1.1";
#include <stdio.h>
#include "../../include/dev.h"
#include "dev.h"
extern int curcol;

newcolor(col)
int col;
{
	if(col < 1 || col > 4){PENSEL; sbn(1);}
	else {PENSEL; sbn(col);}
	curcol = col;
	return(SUCCESS);
}
