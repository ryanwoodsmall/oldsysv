/*	@(#)fabs.c	1.3	*/
/*LINTLIBRARY*/

double
fabs(arg)
double arg;
{
	return((arg < 0)? -arg : arg);
}
