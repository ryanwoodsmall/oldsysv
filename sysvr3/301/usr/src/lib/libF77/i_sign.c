/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:i_sign.c	1.3"
long int i_sign(a,b)
long int *a, *b;
{
long int x;
x = (*a >= 0 ? *a : - *a);
return( *b >= 0 ? x : -x);
}
