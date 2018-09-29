/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)rand_.c	1.4	*/
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

int
irand_()
{
	return(((randx = randx * 1103515245L + 12345)>>16) & 0x7fff);
}

double rand_()
{
	return(irand_()/32768.0);
}
