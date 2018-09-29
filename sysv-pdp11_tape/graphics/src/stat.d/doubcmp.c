static char SCCSID[]="@(#)doubcmp.c	1.1";
/* <:t-5 d:> */
doubcmp(p1,p2) /*double compare used by qsort()*/
double *p1, *p2;
{
	if(*p1 == *p2) return(0);
	else return(*p1<*p2 ? -1:1);
}
