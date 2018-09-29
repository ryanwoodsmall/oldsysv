/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/check.h	1.3"
/*
	global variables for chk* commands
*/

enum	mode {
	YESNO,
	LIST,
	REGEX,
	REGEXR,
	ARITH,
	CMD
};

extern int	anychar;	/* flag to permit non-printing and non-ASCII
				characters input */
extern char	*defaultstr;	/* default value returned for empty input */
extern int	echo;		/* flag for printing of answer */
extern int	errcode;	/* error exit code */
extern int	esc;		/* flag to allow escape to shell */
extern int	force;		/* flag to insist on correct answer */
extern char	*helpstr;	/* this string gives help */
extern char	*helpmsg;	/* this string is help message */
extern char	*killpid;	/* kill this process ID upon quit */
extern char	*quitname;	/* what the quit string is called */
extern char	*quitstr;	/* this string causes a quit */
extern int	trimspace;	/* flag to trim white space from begining and
				end of input */
extern char	*usagestr;	/* custom part of the usage message */

#define	CYCLEDEF	25
#define	LINELEN		74
#define	MAXLINE		256
#define	MAXTEST		10
