/*	@(#)erf_.c	1.2	*/
float erf_(x)
float *x;
{
double erf();

return( erf(*x) );
}
