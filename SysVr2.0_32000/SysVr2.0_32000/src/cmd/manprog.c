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
/*	MANPROG 
	This program calculates and returns the last
	modified date of the give file name.
	NOTE: Months are given from 0 to 11
	therefore to calculate the proper month
	number, 1 must be added to the month returned.  */

char xxxver[] = "@(#)manprog.c	1.4";

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
struct stat buf;
struct tm *tm;
struct tm *localtime();

main(argc,argv)
char **argv;
{
	if(argc<1)
		return(0);
	if(stat(argv[1],&buf)==-1)
		return(0);
	tm = localtime(&buf.st_mtime);
	printf("-rd%d -rm%d -ry%d\n",
		tm->tm_mday,tm->tm_mon,tm->tm_year);
	return(0);
}
