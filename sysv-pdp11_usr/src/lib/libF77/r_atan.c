/*	@(#)r_atan.c	1.2	*/
double r_atan(x)
float *x;
{
double atan();
return( atan(*x) );
}
