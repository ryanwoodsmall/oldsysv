/*	@(#)h_dim.c	1.2	*/
short h_dim(a,b)
short *a, *b;
{
return( *a > *b ? *a - *b : 0);
}
