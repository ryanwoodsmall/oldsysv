/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/doubcmp.c	1.2"
/* <:t-5 d:> */
doubcmp(p1,p2) /*double compare used by qsort()*/
double *p1, *p2;
{
	if(*p1 == *p2) return(0);
	else return(*p1<*p2 ? -1:1);
}
