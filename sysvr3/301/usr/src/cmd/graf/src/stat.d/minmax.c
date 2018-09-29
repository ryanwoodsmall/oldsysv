/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/minmax.c	1.2"
/* <:t-5 d:> */
minmax(buf,ep,min,max) /* return min and max elements from buf */
double *buf, *ep, *min, *max;
{
	for(*min= *max= *buf++; buf<=ep; buf++)
		if(*buf < *min) *min = *buf;
		else if(*buf > *max) *max = *buf;
}
