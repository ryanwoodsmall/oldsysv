/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/todc.h	10.1"
/* Time of Day Clock */

struct seg {
	int :28; unsigned units:4;
	int :28; unsigned tens:4;
	};

struct clock {
	int :28; unsigned test:4;
	int :28; unsigned tenths:4;
	struct seg secs;
	struct seg mins;
	struct seg hours;
	struct seg days;
	int :28; unsigned dayweek:4;
	struct seg months;
	int :28; unsigned years:4;
	int :28; unsigned stop_star:4;
	int :28; unsigned int_stat:4;
};

#define	OTOD		0x00041000L
#define	SBDTOD		((struct clock *)OTOD)
