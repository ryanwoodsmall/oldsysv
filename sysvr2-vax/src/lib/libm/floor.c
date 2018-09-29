/*	@(#)floor.c	1.6	*/
/*LINTLIBRARY*/
/*
 *	floor(x) returns the largest integer (as a double-precision number)
 *	not greater than x.
 *	ceil(x) returns the smallest integer not less than x.
 */

extern double modf();

double
floor(x) 
double x;
{
	double y; /* can't be in register because of modf() below */

	return (modf(x, &y) < 0 ? y - 1 : y);
}

double
ceil(x)
double x;
{
	double y; /* can't be in register because of modf() below */

	return (modf(x, &y) > 0 ? y + 1 : y);
}
