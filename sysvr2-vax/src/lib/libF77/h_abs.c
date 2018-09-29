/*	@(#)h_abs.c	1.2	*/
short h_abs(x)
short *x;
{
if(*x >= 0)
	return(*x);
return(- *x);
}
