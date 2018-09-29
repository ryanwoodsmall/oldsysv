/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/tekinit.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "termodes.h"
#include "errpr.h"
struct tekterm tm={UNSET,UNSET,ALPHA};

tekinit()
{
	extern char *nodename;

	ALPHAMODE;
	if (ioctl(1,TCGETA,&tm.cook) == -1)
		ERRPR0(no terminal);
	tm.raw = tm.cook;
	tm.raw.c_iflag = 0;
	tm.raw.c_oflag = OPOST|ONLCR;
	tm.raw.c_cflag &= ~(PARENB | CSIZE);
	tm.raw.c_cflag |= CS8;
	tm.raw.c_lflag = 0;
	tm.raw.c_cc[4] = 10;
	tm.raw.c_cc[5] = 1;
	getacur(&tm.alphax,&tm.alphay);
	tm.curls = UNSET;  tm.curlw = UNSET; tm.curmode = ALPHA;
	putchar('\n');
}
