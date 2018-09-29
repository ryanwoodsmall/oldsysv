static char SCCSID[]="@(#)minmax.c	1.1";
/* <:t-5 d:> */
minmax(buf,ep,min,max) /* return min and max elements from buf */
double *buf, *ep, *min, *max;
{
	for(*min= *max= *buf++; buf<=ep; buf++)
		if(*buf < *min) *min = *buf;
		else if(*buf > *max) *max = *buf;
}
