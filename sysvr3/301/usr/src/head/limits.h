/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:limits.h	1.1"
/*
        /usr/group Standards File limits.h for System V on 3B2"
*/

#define	ARG_MAX		5120		/* max length of arguments to exec */
#define	CHAR_BIT	8		/* # of bits in a "char" */
#define	CHAR_MAX	127		/* max integer value of a "char" */
#define	CHAR_MIN	-128		/* min integer value of a "char" */
#define	CHILD_MAX	25		/* max # of processes per user id */
#define	CLK_TCK		100		/* # of clock ticks per second */
#define	DBL_DIG		16		/* digits of precision of a "double" */
#define	DBL_MAX		1.79769313486231470e+308 /*max decimal value of a "double"*/
#define	DBL_MIN		4.94065645841246544e-324 /*min decimal value of a "double"*/
#define	FCHR_MAX	1048576		/* max size of a file in bytes */
#define	FLT_DIG		7		/* digits of precision of a "float" */
#define	FLT_MAX		3.40282346638528860e+38 /*max decimal value of a "float" */
#define	FLT_MIN		1.40129846432481707e-45 /*min decimal value of a "float" */
#define	HUGE_VAL	3.40282346638528860e+38 /*error value returned by Math lib*/
#define	INT_MAX		2147483647	/* max decimal value of an "int" */
#define	INT_MIN		-2147483648	/* min decimal value of an "int" */
#define	LINK_MAX	32767		/* max # of links to a single file */
#define	LONG_MAX	2147483647	/* max decimal value of a "long" */
#define	LONG_MIN	-2147483648	/* min decimal value of a "long" */
#define	NAME_MAX	14		/* max # of characters in a file name */
#define	OPEN_MAX	20		/* max # of files a process can have open */
#define	PASS_MAX	8		/* max # of characters in a password */
#define	PATH_MAX	256		/* max # of characters in a path name */
#define	PID_MAX		30000		/* max value for a process ID */
#define	PIPE_BUF	5120		/* max # bytes atomic in write to a pipe */
#define	PIPE_MAX	5120		/* max # bytes written to a pipe in a write */
#define	SHRT_MAX	32767		/* max decimal value of a "short" */
#define	SHRT_MIN	-32767		/* min decimal value of a "short" */
#define	STD_BLK		1024		/* # bytes in a physical I/O block */
#define	SYS_NMLN	9		/* # of chars in uname-returned strings */
#define	UID_MAX		30000		/* max value for a user or group ID */
#define	USI_MAX		4294967296	/* max decimal value of an "unsigned" */
#define	WORD_BIT	32		/* # of bits in a "word" or "int" */

