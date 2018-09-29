/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)gtcfile.c	1.3 */
#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "uust.h"


/*
 *	gtcfile
 */


FILE *Cfp = NULL;
char Cfile[NAMESIZE];


/*
 * get a Cfile descriptor
 * return:
 *	an open file descriptor
 */

FILE *
gtcfile(sys, Grade)
register char *sys;
{
	register int i;
	extern int errno;
	char *strcpy();
	char	sqnm[5];

	DEBUG(4,"gtcfile0: sys - %s\n",sys);
	if( csys(sys, Grade) == NULL){

		DEBUG(4,"gtcfile1: sys - %s\n",sys);
		clscfile();
		if(jobid == 0){
			getseq(sqnm);
			subjob[0] = '@';
			DEBUG(4,"gtcfile2: sqnm - %s\n",sqnm);
		}
		updjb(); sprintf(Cfile, "%c.%.6s%c%.1s%.4d", CMDPRE, sys, Grade, subjob, jobid); 
                DEBUG(4,"gtcfile3: jobid - %d\n",jobid);
		Cfp = fopen(Cfile, "a+");
		ASSERT(Cfp != NULL, "CAN'T OPEN", Cfile, 0);
		setbuf(Cfp, CNULL);
	}else {
		DEBUG(4,"gtcfile4: previous Cfile : %s\n", Cfile);
	}
	return(Cfp);
}

/*   Check to see if a work file has been created for a
 *   given system with a given grade
 */

csys(p, g)
register char *p, g;
{
	char tfile[NAMESIZE];

	sprintf(tfile, "%c.%.6s%c%.1s%.4d", CMDPRE, p, g, subjob, jobid);
	DEBUG(4,"gtcfile - temp: %s || Cfile:",tfile);
	DEBUG(4,"%s\n", Cfile);
	if(strcmp(tfile, Cfile)==SAME)
		return(1);
	return(NULL);
}


/*
 * close cfile
 * return
 *	none
 */
clscfile()
{
	if (Cfp == NULL)
		return;
	fclose(Cfp);
	chmod(Cfile, ~WFMASK & 0777); 
	logent(Cfile, "QUE'D");
/*
	US_CRS(Cfile);
*/
	Cfp = NULL;
	return;
}
