/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:h_sign.c	1.3"
short h_sign(a,b)
short *a, *b;
{
short x;
x = (*a >= 0 ? *a : - *a);
return( *b >= 0 ? x : -x);
}
