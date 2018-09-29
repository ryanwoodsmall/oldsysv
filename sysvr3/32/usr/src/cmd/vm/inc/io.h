/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm:inc/io.h	1.1"

#include        <stdio.h>
#include        "mio.h"
#include        "mess.h"

#define STR_SIZE 512
#define ANY 99
#define DV      87      /* dummy var for exit code */

#ifndef YES
#define YES     1
#endif

#ifndef NO
#define NO      0
#endif

#define FILENO 7
#define FILECOUNT 10
#define FILENAME 15
#define ISDIR   99
#define ISFILE  88
#define RESTART LBUT
#define DONE    LBUT-1
#define MORE    LBUT-2
#define PRE     LBUT-3
#define BUT1    FBUT
#define BUT2    FBUT+1
#define BUT3    FBUT+2
#define BUT4    FBUT+3
#define BUT5    FBUT+4
#define BUT1R LBUT-6
#define BUT2R LBUT-5
#define BUT3R LBUT-4
#define MAXFILE 150
#define CRYPT   70
#define RECRYPT 90
#define REG	64
#define FULL    65
#define STRUCT  66
#define BOTTOMLEVEL     4
#define LMAX    60
#define LSIZE   150
#define GCOUNT  9

#define PROCESS 0
#define THROW_OUT -1
#define EDIT	-2
#define LINENO  16
#define LINELG  48
#define MAXPGS  102
#define MXC_PSL 48
#define MXC_PLL 80
#define MXL_PSP 15
#define MXL_PLP 13

#define	NOTSET	0
#define ENDDOC	1
#define ASCII	2

struct listing  {
	char name[15];
	char members[100];
};

char *ctime();

struct timest   {
	long time;
	char name[15];
};
#ifndef MAIL
#define	NEWS	16	
#define	MAIL	17	
#endif
