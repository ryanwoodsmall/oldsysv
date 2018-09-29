/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/log.c	1.1"

#include "varargs.h"

#include "lpsched.h"

/**
 ** open_logfile() - OPEN FILE FOR LOGGING MESSAGE
 ** close_logfile() - CLOSE SAME
 **/

FILE			*open_logfile (name)
	char			*name;
{
	register char		*path;

	register FILE		*fp;


#if	defined(MALLOC_3X) || defined(I_AM_LPSCHED_JR)
	/*
	 * Don't rely on previously allocated pathnames.
	 */
#endif
	path = makepath(getspooldir(), LOGSDIR, name, (char *)0);
	fp = fopen(path, "a", MODE_NOREAD);
	free (path);
	return (fp);
}

void			close_logfile (fp)
	FILE			*fp;
{
	fclose (fp);
	return;
}

#if	!defined(I_AM_LPSCHED_JR)

static void		log();

/**
 ** fail() - LOG MESSAGE AND EXIT (ABORT IF DEBUGGING)
 **/

/*VARARGS0*/
void			fail (va_alist)
	va_dcl
{
	va_list			ap;
    
	va_start (ap);
	log (ap);
	va_end (ap);

	if (debug)
		(void)abort ();
	else
		exit (1);
	/*NOTREACHED*/
}

/**
 ** note() - LOG MESSAGE
 **/

/*VARARGS0*/
void			note (va_alist)
	va_dcl
{
	va_list			ap;

	va_start (ap);
	log (ap);
	va_end (ap);
	return;
}

/**
 ** log() - LOW LEVEL ROUTINE THAT LOGS MESSSAGES
 **/

static void		log (ap)
	va_list			ap;
{
	int			code,
				close_it;

	FILE			*fp;


	if (!am_in_background) {
		fp = stdout;
		close_it = 0;
	} else {
		if (!(fp = open_logfile("lpsched")))
			return;
		close_it = 1;
	}

	code = va_arg(ap, int);
	if (am_in_background) {
		long			now;

		time (&now);
		fprintf (fp, "%24.24s: ", ctime(&now));
	}
	vfprintf (fp, logmsg[code], ap);

	if (close_it)
		close_logfile (fp);
	else
		fflush (fp);

	return;
}

#endif		/* !defined(I_AM_LPSCHED_JR) */
