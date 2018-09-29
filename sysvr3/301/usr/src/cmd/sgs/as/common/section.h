/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/section.h	1.2"

/*
 * output section information
 */

struct scninfo
{
	FILE		*s_fd;		/* file pointer */
	long		s_typ;		/* section symbol type */
	long		s_any;		/* >0 => code generated */
	long		s_up;		/* !=0 => round size */
	long		s_buf[TBUFSIZ];/* code output buffer */
	long		s_cnt;		/* number of elements in buffer */
};
