/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/errlst.c	1.13.1.1"
/*LINTLIBRARY*/
char	*sys_errlist[] = {
	"Error 0",
	"Not owner",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"I/O error",
	"No such device or address",
	"Arg list too long",
	"Exec format error",
	"Bad file number",
	"No child processes",
	"No more processes",
	"Not enough space",
	"Permission denied",
	"Bad address",
	"Block device required",
	"Device busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Argument out of domain",
	"Result too large",
	"No message of desired type",
	"Identifier removed",
	"Channel number out of range",
	"Level 2 not synchronized",
	"Level 3 halted",
	"Level 3 reset",
	"Link number out of range",
	"Protocol driver not attached",
	"No CSI structure available",
	"Level 2 halted",
	"Deadlock situation detected/avoided",
	"No record locks available",
	"Error 47",
	"Error 48",
	"Error 49",
	"Bad exchange descriptor",		/* 50 */
	"Bad request descriptor",		/* 51 */
	"Message tables full",			/* 52 */
	"Anode table overflow",			/* 53 */
	"Bad request code",			/* 54 */
	"Invalid slot",				/* 55 */
	"File locking deadlock",		/* 56 */
	"Bad font file format",			/* 57 */
	"Error 58",
	"Error 59",
	"Not a stream device",			/* 60 */
	"No data available",
	"Timer expired",
	"Out of stream resources",
	"Machine is not on the network",
	"Package not installed",
	"Object is remote",
	"Link has been severed",
	"Advertise error",
	"Srmount error",
	"Communication error on send",		/* 70 */
	"Protocol error",
	"Error 72",
	"Error 73",
	"Multihop attempted",
	"Error 75",
	"Error 76",
	"Not a data message",			/* 77 */
	"Error 78",
	"Error 79",
	"Name not unique on network",
	"File descriptor in bad state",
	"Remote address changed",
	"Can not access a needed shared library",	/* 83 */
	"Accessing a corrupted shared library",
	".lib section in a.out corrupted",
	"Attempting to link in more shared libraries than system limit",
	"Can not exec a shared library directly"

};
int	sys_nerr = { sizeof(sys_errlist)/sizeof(sys_errlist[0]) };

