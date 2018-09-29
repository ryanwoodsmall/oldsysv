/*	@(#)d_sign.c	1.3	*/
double d_sign(a,b)
double *a, *b;
{
double x;
	x = *a;
	if (x <= 0)
		x = -x;
	if (*b < 0)
		x = -x;
	return (x);
/*x = (*a >= 0 ? *a : - *a);
 *return( *b >= 0 ? x : -x);
*/
}
