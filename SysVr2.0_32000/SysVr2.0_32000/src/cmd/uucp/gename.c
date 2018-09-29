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
/* @(#)gename.c	1.5 */
#include "uucp.h"


 /* generate file name
  *	pre	-> file prefix
  *	sys	-> system name
  *	grade	-> service grade 
  *	file	-> buffer to return filename must be of size DIRSiz+1
  * return:
  *	none
  */
gename(pre, sys, grade, file)
char pre, *sys, grade, *file;
{
	char sqnum[5];

	getseq(sqnum);
	updjb(); sprintf(file, "%c.%.6s%c%.1s%.4s", pre, sys, grade, subjob, sqnum); 
	DEBUG(4, "file - %s\n", file);
	return;
}


#define SLOCKTIME 10L
#define SLOCKTRIES 5
#define SEQLEN 4

 /*
  * get next sequence number
  *	snum	-> array to return sequence # in
  *		-> must be 5 chars long.
  * returns:  
  *	FAIL	-> can't get sequence # and can't create a new one
  *	 0  	-> Success snum contains Ascii 3 digit sequence #
  */
getseq(snum)
char *snum;
{
	register FILE *fp;
	register int i;
	int n;
	unsigned sleep();

	for (i = 1; i < SLOCKTRIES; i++) {
		if (!ulockf( SEQLOCK, SLOCKTIME))
			break;
		sleep(5);
	}

	ASSERT(i < SLOCKTRIES, "CAN NOT GET", SEQLOCK, 0);

	if ((fp = fopen(SEQFILE, "r")) != NULL) {

		/*
		 * read sequence number file
		 */
		fscanf(fp, "%4d", &n);
		fp = freopen(SEQFILE, "w", fp);
		ASSERT(fp != NULL, "CAN NOT OPEN", SEQFILE, 0);
		chmod(SEQFILE, 0666);
	} else {

		/* 
		 * can not read file - create a new one
		 */
		if ((fp = fopen(SEQFILE, "w")) == NULL)
			
			return(FAIL);
		chmod(SEQFILE, 0666);
		n = 0;
	}

	sprintf(snum, "%.4d", ++n);
	jobid = n;
	fprintf(fp, "%s", snum);
	fclose(fp);
	rmlock(SEQLOCK);
	return(0);
}
