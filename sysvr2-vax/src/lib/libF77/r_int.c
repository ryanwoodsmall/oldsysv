/*	@(#)r_int.c	1.2	*/
double r_int(x)
float *x;
{
double floor();

return( (*x>0) ? floor(*x) : -floor(- *x) );
}
