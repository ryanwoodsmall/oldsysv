/*	@(#)i_dnnt.c	1.2	*/
long int i_dnnt(x)
double *x;
{
double floor();

return( (*x)>=0 ?
	floor(*x + .5) : -floor(.5 - *x) );
}
