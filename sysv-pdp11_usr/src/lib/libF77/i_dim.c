/*	@(#)i_dim.c	1.2	*/
long int i_dim(a,b)
long int *a, *b;
{
return( *a > *b ? *a - *b : 0);
}
