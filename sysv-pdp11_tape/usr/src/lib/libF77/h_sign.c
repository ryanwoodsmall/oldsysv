/*	@(#)h_sign.c	1.2	*/
short h_sign(a,b)
short *a, *b;
{
short x;
x = (*a >= 0 ? *a : - *a);
return( *b >= 0 ? x : -x);
}
