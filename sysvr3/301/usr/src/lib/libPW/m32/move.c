/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libPW:m32/move.c	1.3"

char *
move(a,b,n)
register char *a, *b;
int n;
{
	register i;
	char *rs;

	rs = b;
	for (i=0; i<n; i++)
		*b++ = *a++;
	return(rs);
}
