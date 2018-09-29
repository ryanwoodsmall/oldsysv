/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:signal_.c	1.4"

signal_(sigp, procp,L)
long L;
int *sigp, (*procp)();
{
return( signal(*sigp, *procp) );
}
