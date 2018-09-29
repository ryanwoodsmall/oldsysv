/*	@(#)d_tanh.c	1.2	*/
double d_tanh(x)
double *x;
{
double tanh();
return( tanh(*x) );
}
