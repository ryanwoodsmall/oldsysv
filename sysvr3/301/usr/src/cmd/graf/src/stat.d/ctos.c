/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/ctos.c	1.2"
char *ctos(c) /* convert char to string */
char c;
{
	static char s[2] = { '\0','\0' };
	s[0] = c;
	return(s);
}
