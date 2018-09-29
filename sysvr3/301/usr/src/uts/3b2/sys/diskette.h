/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/diskette.h	10.2"
#define FORMAT 'f'		/* ioctl flag for format */
#define VERIFY 'v'		/* mode is 'v' to verify, 0 otherwise */

/* should be in fcntl.h */
#define O_FORMAT 0200		/* open flag for format */

struct fmtstruct
{
	char mode;
	int passcnt;
} ;
