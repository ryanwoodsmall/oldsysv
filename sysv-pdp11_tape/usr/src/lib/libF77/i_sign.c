/*	@(#)i_sign.c	1.2	*/
long int i_sign(a,b)
long int *a, *b;
{
long int x;
x = (*a >= 0 ? *a : - *a);
return( *b >= 0 ? x : -x);
}
