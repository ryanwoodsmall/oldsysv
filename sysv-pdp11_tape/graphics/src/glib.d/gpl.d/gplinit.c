static char SCCSID[]="@(#)gplinit.c	1.1";
/* <: t-5 d :> */
#include<stdio.h>
#include "../../../include/gpl.h"
#include "../../../include/util.h"
gplinit(fpo)
FILE *fpo; 
{	int i[];
	gplcomment(fpo,i,0);
}
