/*	@(#)i_nint.c	1.2	*/
long int i_nint(x)
float *x;
{
double floor();

return( (*x)>=0 ?
	floor(*x + .5) : -floor(.5 - *x) );
}
