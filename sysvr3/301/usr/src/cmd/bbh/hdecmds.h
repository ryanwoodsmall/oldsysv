/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bbh:hdecmds.h	1.3"

/* exit codes */
#define NORMEXIT	0
#define INVEXIT		1	/* invalid command invocation */
#define ERREXIT		1	/* or fatal error incurred */
#define BADBEXIT	2	/* bad block reports found in log(s) */

#define HDEDLVAL	0x4a11badb
#define HDEDLVER	1

/* hard disk error record: a summary of reports of errors
 * for a given block. */
struct hderecord {
	daddr_t	blkaddr;
	uint	readtype : 2,
		severity : 2,
		badrtcnt : 12,
		bitwidth : 6,
		reportct : 10;
	time_t	firsttime;
	time_t	lasttime;
};
#define MAXBWIDTH	63
#define MAXRPTCNT	1023
#define MAXBRTCNT	4095

/* hard disk error disk log format: must 512 bytes long */
#define NUMRCRDS	31
union hdedlog {
	char hdedlsec[512];
	struct hdedl {
		long	l_valid;
		short	l_vers;
		short	logentct;
		time_t	createdt;
		time_t	lastchg;
		struct hderecord errlist[NUMRCRDS];
	} hdedl;
};
