/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/fs/s5dir.h	10.1"
#ifndef	DIRSIZ
#define	DIRSIZ	14
#endif
struct	direct
{
	ushort	d_ino;
	char	d_name[DIRSIZ];
};
