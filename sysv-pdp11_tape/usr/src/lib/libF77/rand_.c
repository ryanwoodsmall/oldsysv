/*	@(#)rand_.c	1.3	*/
/*
Uniform random number generator. 
Linear congruential generator, suitable for 32 bit machines;
multiplication is mod 2**31
*/

static	long	randx = 1;

srand_(x)	/* subroutine to set seed */
long *x;
{
randx = *x;
}




double rand_()
{
double ldexp();
return(ldexp((double)(((randx = randx*1103515245 + 12345)>>7) & 077777777), -24));
}
