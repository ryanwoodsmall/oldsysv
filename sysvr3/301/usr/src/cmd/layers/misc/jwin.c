/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)misc:jwin.c	2.4"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
#include <sys/jioctl.h>

struct jwinsize win;
main(argc,argv)
char *argv[];
{
	if (argc != 1) {
		fprintf(stderr,"usage: jwin\n");
		exit(1);
	}

	if (ioctl(0, JMPX, 0) == -1) {
		fprintf(stderr,"jwin: not mpx\n");
		exit(1);
	} else {
		ioctl(0, JWINSIZE, &win);
		printf("bytes:\t%d %d\n", win.bytesx, win.bytesy);
		if (win.bitsx != 0 || win.bitsy != 0)
			printf("bits:\t%d %d\n", win.bitsx, win.bitsy);
		exit(0);
	}
}
