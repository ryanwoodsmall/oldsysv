/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:d_lg10.c	1.3"
#define log10e 0.43429448190325182765

double d_lg10(x)
double *x;
{
double log();

return( log10e * log(*x) );
}
