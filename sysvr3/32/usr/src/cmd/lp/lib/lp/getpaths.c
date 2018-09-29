/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/getpaths.c	1.5"

#include "lp.h"

char			*Lp_Spooldir,
			*Lp_Admins,
			*Lp_Bin,
			*Lp_FIFO,
			*Lp_Logs,
#if	defined(LPSCHED_JR)
			*Lp_Lpsched_Jr,
#endif
			*Lp_ReqLog,
			*Lp_Model,
			*Lp_Private_FIFOs,
			*Lp_Public_FIFOs,
			*Lp_Requests,
			*Lp_Schedlock,
			*Lp_Slow_Filter,
			*Lp_System,
			*Lp_Temp,
			*Lp_Users;

char			*Lp_A,
			*Lp_A_Classes,
			*Lp_A_Forms,
			*Lp_A_Interfaces,
			*Lp_A_Logs,
			*Lp_A_Printers,
			*Lp_A_PrintWheels,
			*Lp_A_Filters;

extern void		free();

void			getpaths ()
{
	if (Lp_Spooldir)
		return;

#define MAKEPATH1(X)	makepath(Lp_Spooldir, X, (char *)0)
#define MAKEPATH2(X,Y)	makepath(Lp_Spooldir, X, Y, (char *)0)

	Lp_Spooldir = getspooldir();
	Lp_Admins = MAKEPATH1(ADMINSDIR);
	Lp_Bin = MAKEPATH1(BINDIR);
		Lp_Slow_Filter = makepath(Lp_Bin, SLOWFILTER, (char *)0);
#if	defined(LPSCHED_JR)
		Lp_Lpsched_Jr = makepath(Lp_Bin, LPSCHED_JR, (char *)0);
#endif
	Lp_Logs = MAKEPATH1(LOGSDIR);
		Lp_ReqLog = makepath(Lp_Logs, REQLOGFILE, (char *)0);
	Lp_Model = MAKEPATH1(MODELSDIR);
	Lp_FIFO = MAKEPATH2(FIFOSDIR, FIFO);
	Lp_Private_FIFOs = MAKEPATH2(FIFOSDIR, PRIVFIFODIR);
	Lp_Public_FIFOs = MAKEPATH2(FIFOSDIR, PUBFIFODIR);
	Lp_Requests = MAKEPATH1(REQUESTSDIR);
	Lp_Schedlock = MAKEPATH1(SCHEDLOCK);
	Lp_System = MAKEPATH1(SYSTEMDIR);
	Lp_Temp = MAKEPATH1(TEMPDIR);
	Lp_Users = MAKEPATH1(USERSFILE);

#undef MAKEPATH1
#undef MAKEPATH2

	return;
}

void			getadminpaths (admin)
	char			*admin;
{
	if (!Lp_Spooldir)
		getpaths ();

	if (Lp_A) {
		free (Lp_A);
		free (Lp_A_Classes);
		free (Lp_A_Forms);
		free (Lp_A_Interfaces);
		free (Lp_A_Logs);
		free (Lp_A_Printers);
		free (Lp_A_PrintWheels);
		free (Lp_A_Filters);
	}

#define MAKEPATH1(X)	makepath(Lp_A, X, (char *)0)

	Lp_A = makepath(Lp_Admins, admin, (char *)0);
	Lp_A_Classes = MAKEPATH1(CLASSESDIR);
	Lp_A_Forms = MAKEPATH1(FORMSDIR);
	Lp_A_Interfaces = MAKEPATH1(INTERFACESDIR);
	Lp_A_Logs = MAKEPATH1(LOGSDIR);
	Lp_A_Printers = MAKEPATH1(PRINTERSDIR);
	Lp_A_PrintWheels = MAKEPATH1(PRINTWHEELSDIR);
	Lp_A_Filters = MAKEPATH1(FILTERTABLE);

#undef	MAKEPATH1

	return;
}

/**
 ** getprinterfile() - BUILD NAME OF PRINTER FILE
 **/

char			*getprinterfile (name, component)
	char			*name,
				*component;
{
	register char		*path;

	if (!name)
		return (0);

	if (!Lp_A_Printers) {
		getadminpaths (LPUSER);
		if (!Lp_A_Printers)
			return (0);
	}

	path = makepath(Lp_A_Printers, name, component, (char *)0);

	return (path);
}

/**
 ** getclassfile() - BUILD NAME OF CLASS FILE
 **/

char			*getclassfile (name)
	char			*name;
{
	register char		*path;

	if (!name)
		return (0);

	if (!Lp_A_Classes) {
		getadminpaths (LPUSER);
		if (!Lp_A_Classes)
			return (0);
	}

	path = makepath(Lp_A_Classes, name, (char *)0);

	return (path);
}

/**
 ** getfilterfile() - BUILD NAME OF FILTER TABLE FILE
 **/

char			*getfilterfile (table)
	char			*table;
{
	register char		*path;


	if (!table)
		table = FILTERTABLE;

	if (!Lp_Admins) {
		getpaths ();
		if (!Lp_Admins)
			return (0);
	}

	path = makepath(Lp_Admins, LPUSER, table, (char *)0);

	return (path);
}
