/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm:xx/vmaster.c	1.4"

#include <stdio.h>
#include <time.h>

#define PATHSIZ		128
#define MAXARGS		25	
#define NUMARGS		4

static char Interpreter[] = "fmli";	
static char *setbanner();

main(argc, argv)
int argc;
char *argv[];
{
	char *Objlist[MAXARGS];
	char introbuf[PATHSIZ];
	char aliasbuf[PATHSIZ];
	register int i;
	char *getenv();
	char *vmsys;

	if (getenv("VPID") != NULL) {
		fprintf(stderr, "\r\nYou already have FACE running.\r\n");
		exit(1);
	}
	vmsys = getenv("VMSYS");
	sanity_check();		/* make sure environment is set properly */

	sprintf(introbuf, "%s%s", vmsys, "/OBJECTS/Init.vm");
	sprintf(aliasbuf, "%s%s", vmsys, "/pathalias");
	Objlist[0] = Interpreter;
	Objlist[1] = "-i";
	Objlist[2] = introbuf; 
	Objlist[3] = "-a";
	Objlist[4] = aliasbuf;
	if (argc == 1) {
		/*
		 * Use the Office Menu as the default if no arguments
		 */
		char objbuf[PATHSIZ];

		sprintf(objbuf, "%s%s", vmsys, "/OBJECTS/Menu.office");
		Objlist[NUMARGS + 1] = objbuf;
		Objlist[NUMARGS + 2] = NULL;
	}
	else {
		/*
		 * Arguements to vmaster are Object paths
		 */
		for (i = 1; i < argc && (i < (MAXARGS - 3)); i++)
			Objlist[i + NUMARGS] = argv[i];
		Objlist[i + NUMARGS] = (char *) NULL;
	}
	putenv(setbanner());
	putenv("VMFMLI=true");
	execvp(Interpreter, Objlist);
}

static
sanity_check()
{
	char	*home;
	char	pref[PATHSIZ];
	char	tmp[PATHSIZ];
	char	*getenv();

	home = getenv("HOME");
	sprintf(pref, "%s/pref", home);
	sprintf(tmp, "%s/tmp", home);
	if (access(pref, 07))  {
		fprintf(stderr, "Sorry, you are not properly defined as a FACE user.\nPlease have your system administrator run:\n     sysadm usrmgmt\nunder the FACE management menus,\nbecause you are missing your pref directory\n");
		exit(1);
	}
	if (access(tmp, 07)) {
		fprintf(stderr, "Sorry, you are not properly defined as a FACE user.\nPlease have your system administrator run:\n     sysadm usrmgmt\nunder the FACE management menus,\nbecause you are missing your tmp directory\n");
		exit(1);
	}
}

static char *
setbanner()
{
	static char datebuf[BUFSIZ];
	register struct tm	*t;
	char	*ctime();
	static char	*day[] = {
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
	};
	static char	*month[] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December",
	};
	long	cur_time;

	cur_time = time( 0L );
	t = localtime(&cur_time);
	sprintf(datebuf, "VMBANNER=AT&T FACE - %s %s %d, %4d",
		 day[t->tm_wday], month[t->tm_mon],
		 t->tm_mday, t->tm_year + 1900);
	return(datebuf);
}
