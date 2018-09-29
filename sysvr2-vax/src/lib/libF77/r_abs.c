/*	@(#)r_abs.c	1.2	*/
double r_abs(x)
float *x;
{
if(*x >= 0)
	return(*x);
return(- *x);
}
