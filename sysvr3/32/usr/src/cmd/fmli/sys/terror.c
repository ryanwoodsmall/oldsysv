/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/terror.c	1.5"

#include	<stdio.h>
#include	<errno.h>
#include	"wish.h"
#include	"message.h"
#include	"vtdefs.h"
#include	"terror.h"
#include	"retcodes.h"

extern char	*Errlist[];
extern char	*What[];
extern int	Vflag;

void
_terror(sev, what, name, file, line)
int	sev;	/* flags to see if we should log or exit */
int	what;	/* What we were trying to do */
char	*name;	/* What we were trying to do it to */
char	*file;	/* __FILE__ */
int	line;	/* __LINE__ */
{
	if (errno == ENOTTY)
		errno = 0;
	notify(what);
	if (Vflag && (sev & TERR_LOG))
		log(sev, what, name, file, line);
	if (sev & TERR_EXIT)
		exit(R_ERR);
	errno = 0;
}

static
notify(what)
int	what;
{
	register char	*mymess;
	register int	length;
	register char	*screen;
	register int	mywin;
	char	messbuf[PATHSIZ];
	extern bool	Use_errno[];
	char	*push_win();

	if ((mymess = What[what]) == nil && Errlist[errno] == nil)
		return;
	if (mymess == nil)
		mymess = "error";

	length = strlen(What[what]);
	if (Use_errno[what])
		length += strlen(Errlist[errno]) + 2;
	/* to ensure '\0' termination of string */
	messbuf[PATHSIZ - 1] = '\0';
	strncpy(messbuf, mymess, PATHSIZ - 1);
	if (Use_errno[what] && Errlist[errno] != nil) {
		strncat(messbuf, ": ", PATHSIZ - 1 - strlen(messbuf));
		strncat(messbuf, Errlist[errno], PATHSIZ - 1 - strlen(messbuf));
	}
	mess_temp(messbuf);
}

#define LOGFILE		0
#define TERMINAL	1
#define MAILADM		2

/*
 * FACE application ONLY ....  log problems in the TERRLOG file
 * and/or send mail
 */
static
log(sev, what, name, file, line)
int	sev;
int	what;
char	*name;
char	*file;
int	line;
{
	char	path[PATHSIZ];
	register int	method;
	long	t;
	register FILE	*fp;
	extern char	*Oasys;
	extern char	*Progname;
	extern char	*sys_errlist[];
	char	*getenv();
	long	time();

	/*
	 * construct path of error log file
	 */
	method = LOGFILE;
	if (name == NULL)
		name = nil;
	strcat(strcpy(path, Oasys), TERRLOG);
	if ((fp = fopen(path, "a")) == NULL && errno == EMFILE) {
		close(4);
		fp = fopen(path, "a");
	}
	if (fp == NULL)
		if ((fp = popen("mail $LOGNAME", "w")) == NULL) {
			method = TERMINAL;
			fp = stderr;
		}
		else
			method = MAILADM;
	(void) time(&t);
	setbuf(fp, NULL);
	fprintf(fp, "%16.16s %-8s %-14s %-14s %3d %s%-*s %-24s %s\n", ctime(&t),
		getenv("LOGNAME"), Progname, file, line,
		(sev & TERR_EXIT) ? "(FATAL)" : nil,
		(sev & TERR_EXIT) ? 17 : 24,
		What[what], sys_errlist[errno], name);
	if (method == LOGFILE)
		fclose(fp);
	else if (method == MAILADM)
		pclose(fp);
}
