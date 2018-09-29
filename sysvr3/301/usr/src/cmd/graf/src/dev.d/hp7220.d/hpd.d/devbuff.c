/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/devbuff.c	1.2"
#include <stdio.h>
#include "hpd.h"
#include "dev.h"
extern struct device dv;
extern char *nodename;

devbuff()
{
	static FILE *devname;
	FILE *fopen();
	static int openflag = 1;
	char *ptr, *ttyname();

	if (openflag) {
		openflag = 0;
		if ((ptr = ttyname(fileno(stdout))) == NULL) {
			dv.hbuff = FALSE;
			return;
		}
		else if ((devname = fopen(ptr, "r")) == NULL) {
			devfinal();
			ERRPR0(can not open tty for reading);
			exit(1);
		}
		else {
			SETOMODE;
			SETHSHK1;
		}
	}
	putc(SHKENAB,stdout);
	getc(devname);
}
