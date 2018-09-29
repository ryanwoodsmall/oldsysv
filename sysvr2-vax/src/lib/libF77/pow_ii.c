/*	@(#)pow_ii.c	1.3	*/

long int pow_ii(ap, bp)
long int *ap, *bp;
{
long int pow, x, n;
if(n = *bp) { if (n<0) return(0);
	else {
	pow=1;
	x = *ap;
	for( ; ; )
		{
		if(n & 01)
			pow *= x;
		if(n >>= 1)
			x *= x;
		else
			return(pow);
		}
	}
     }
else return(1);
}
