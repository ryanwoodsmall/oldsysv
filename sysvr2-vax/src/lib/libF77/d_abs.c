/*	@(#)d_abs.c	1.2	*/
double d_abs(x)
double *x;
{
if(*x >= 0)
	return(*x);
return(- *x);
}
